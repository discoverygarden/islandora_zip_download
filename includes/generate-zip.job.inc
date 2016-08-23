<?php
/**
 * @file
 * Gearman job definition and helpers.
 */

class IslandoraZipDownloadLimitException extends Exception {}
class IslandoraZipDownloadSourceLimitException extends IslandoraZipDownloadLimitException {
  protected $measure;
  protected $limit;

  /**
   * Constructor.
   */
  public function __construct($measure, $limit) {
    parent::__construct(t('Source material size limit met or exceeded: !measure/!limit', array(
      '!measure' => $measure,
      '!limit' => $limit,
    )));
    $this->measure = $measure;
    $this->limit = $limit;
  }
}
class IslandoraZipDownloadExportLimitException extends IslandoraZipDownloadLimitException {
  protected $measure;
  protected $limit;

  /**
   * Constructor.
   */
  public function __construct($measure, $limit) {
    parent::__construct(t('Export size limit met or exceeded: !measure/!limit', array(
      '!measure' => $measure,
      '!limit' => $limit,
    )));
    $this->measure = $measure;
    $this->limit = $limit;
  }
}

/**
 * Generate the ZIP file and trigger rules events accordingly.
 *
 * @param array $info
 *   An associative array containing:
 *   - user_id: The user exporting the ZIP.
 *   - mimes: An associative array containing:
 *     - all: The array of all the selected MIME-types.
 *     - exclude: The array of MIME-types to exclude.
 *     - filtered: 'all' less 'exclude'
 *   - dsids: An associative array containing:
 *     - exclude: The datastream IDs to exclude.
 *   - pids: An associative array containing:
 *     - base: The PIDs from which to start iteration.
 *     - exclude: The PIDs configured to be excluded.
 *     - filtered: 'base' less 'exclude'
 */
function islandora_zip_download_generate_zip($info) {
  // Need to act as the given user, so as not to expose resources to which
  // the user should not have access.
  global $user;
  $user = user_load($info['user_id']);
  drupal_static_reset('islandora_get_tuque_connection');

  $zip_file_uri = file_create_filename('islandora_zip_download', 'temporary://');
  $zip_file_path = drupal_realpath($zip_file_uri);
  $zip = new ZipArchive();
  $stats = array(
    'count' => 0,
    'size' => array(
      'source' => 0,
      'compressed' => 0,
    ),
  );

  $base_limits = variable_get('islandora_zip_download_size_limits', array(
    'scale' => 30,
    'source_limit' => 0,
    'export_limit' => 0,
    'split_size' => 0,
    'split_executable' => 'split',
  ));
  $map_to_gib = function ($num) use ($base_limits) {
    return $num * (1 << $base_limits['scale']);
  };
  $size_limits = array_map($map_to_gib, $base_limits);
  $size_constrained = FALSE;

  $check_stats = function () use (&$stats, $size_limits) {
    // Throw exceptions if we meet or exceed any limits.
    if ($size_limits['source_limit'] > 0 && $stats['size']['source'] >= $size_limits['source_limit']) {
      throw new IslandoraZipDownloadSourceLimitException($stats['size']['source'], $size_limits['source_limit']);
    }
    elseif ($size_limits['export_limit'] > 0 && $stats['size']['compressed'] >= $size_limits['export_limit']) {
      throw new IslandoraZipDownloadExportLimitException($stats['size']['compressed'], $size_limits['export_limit']);
    }
  };

  $write_file_to_zip = function ($actual_file, $destination_name = NULL) use ($zip, $zip_file_path, &$stats, $check_stats) {
    $zip->open($zip_file_path, ZipArchive::CREATE);
    $zip->addFile(drupal_realpath($actual_file), $destination_name);
    $zip->close();
    $stats['count'] += 1;
    $stats['size']['source'] += filesize(drupal_realpath($actual_file));
    $stats['size']['compressed'] = filesize($zip_file_path);
    $check_stats();
  };
  $datastream_filename = function (AbstractDatastream $datastream) {
    // XXX: Copy/pasta from islandora_view_datastream(), when type is
    // 'download'.
    module_load_include('inc', 'islandora', 'includes/mimetype.utils');

    // Browsers will not append all extensions.
    $extension = '.' . islandora_get_extension_for_mimetype($datastream->mimetype);
    // Prevent adding on a duplicate extension.
    $label = $datastream->label;
    $extension_length = strlen($extension);
    $duplicate_extension_position = strlen($label) > $extension_length ?
      strripos($label, $extension, -$extension_length) :
      FALSE;
    $filename = $label;
    if ($duplicate_extension_position === FALSE) {
      $filename .= $extension;
    }
    return $filename;
  };
  $write_datastream_to_zip = function (AbstractDatastream $datastream, $dir) use ($write_file_to_zip, $datastream_filename) {
    try {
      $tempfile = drupal_tempnam('temporary://', 'islandora_zip_download.zip');
      $datastream->getContent($tempfile);
      $filename = $datastream_filename($datastream);
      $write_file_to_zip($tempfile, "$dir/$filename");
    }
    finally {
      drupal_unlink($tempfile);
    }
  };
  $unfiltered = function (AbstractDatastream $datastream) use ($info) {
    $id = $datastream->id;
    $mime = $datastream->mimetype;

    return !in_array($id, $info['dsids']['exclude']) && (
        empty($info['mimes']['all']) || in_array($datastream->mimetype, $info['mimes']['filtered']));
  };
  $iterator = new RecursiveIteratorIterator(new IslandoraZipDownloadZipIterator($info['pids']['filtered'], $info['pids']['exclude']), RecursiveIteratorIterator::SELF_FIRST);

  try {
    foreach ($iterator as $pid) {
      try {
        $object = islandora_object_load($pid);
      }
      catch (RepositoryException $e) {
        watchdog_exception('islandora_zip_download', $e, 'Failed to load @pid.', array('@pid' => $pid), WATCHDOG_WARNING);
      }

      $object_dir = $iterator->getCurrentName();
      foreach ($object as $datastream) {
        try {
          if (!$unfiltered($datastream)) {
            continue;
          }
        }
        catch (RepositoryException $e) {
          watchdog_exception('islandora_zip_download', $e, 'Failed to test @pid/@dsid for inclusion in ZIP download.', array(
            '@pid' => $pid,
            '@dsid' => $datastream->id,
          ));
          continue;
        }

        try {
          $write_datastream_to_zip($datastream, $object_dir);
        }
        catch (IslandoraZipDownloadLimitException $e) {
          throw $e;
        }
        catch (RepositoryException $e) {
          watchdog_exception('islandora_zip_download', $e, 'Failed to acquire @pid/@dsid for the ZIP download.', array(
            '@pid' => $pid,
            '@dsid' => $datastream->id,
          ));
        }
        catch (Exception $e) {
          watchdog_exception('islandora_zip_download', $e, 'Unexpected exception when adding @pid/@dsid to ZIP.', array(
            '@pid' => $pid,
            '@dsid' => $datastream->id,
          ));
        }
      }
    }
  }
  catch (IslandoraZipDownloadLimitException $e) {
    $size_constrained = TRUE;
    watchdog_exception('islandora_zip_download', $e, 'Hit size limit generating ZIP file. Generation parameters: @params', array('@params' => print_r($info, TRUE)), WATCHDOG_NOTICE);
  }

  if ($stats['count'] === 0) {
    rules_invoke_event('islandora_zip_download_zip_empty', $info);
  }
  else {
    $split_size = $size_limits['split_size'];
    if ($split_size > 0 && $stats['size']['compressed'] > $split_size) {
      module_load_include('inc', 'uuid');
      $directory = file_default_scheme() . '://islandora_zip_download/' . uuid_generate();
      file_prepare_directory($directory, FILE_CREATE_DIRECTORY);
      $real_dir = drupal_realpath($directory);
      $prefix = 'zip';
      $dir_prefix = "$real_dir/$prefix";
      $command_components = array(
        $base_limits['split_executable'],
        "--bytes={$split_size}",
        '--numeric-suffixes',
        '--suffix-length=4',
        $zip_file_path,
        $dir_prefix,
      );
      exec(implode(' ', $command_components));
      $map_to_uri = function ($filename) use ($directory) {
        $basename = basename($filename);
        return "$directory/$basename";
      };
      $split = glob($dir_prefix . '*');
      $uris = array_map($map_to_uri, $split);
      $output = 'download.zip';
      $array_to_string = function (array $input, $eol = "\n") {
        return implode($eol, $input) . $eol;
      };
      $reassembly_bat = file_unmanaged_save_data($array_to_string(array(
        '# Place this file beside all the partial ZIP downloads, and on execution, it will',
        '# combine all the component parts into the decompressable ZIP file.',
        format_string('copy /b !files !output', array(
          '!files' => implode('+', $split),
          '!output' => $output,
        )),
      ), "\r\n"), "$directory/reassemble.bat");
      $reassembly_sh = file_unmanaged_save_data($array_to_string(array(
        '# Place this file beside all the partial ZIP downloads, and on execution, it will',
        '# combine all the component parts into the decompressable ZIP file.',
        format_string('cat !prefix* > !output', array(
          '!prefix' => $prefix,
          '!output' => $output,
        )),
      )), "$directory/reassemble.sh");
      $uris[] = $reassembly_bat;
      $uris[] = $reassembly_sh;
    }
    else {
      $uris = array(file_unmanaged_move($zip_file_uri));
    }
    $map_uri_to_md5line = function ($uri) {
      return format_string('!hash *!file', array(
        '!hash' => md5_file($uri),
        '!file' => drupal_basename($uri),
      ));
    };
    $map_uri_to_sha1line = function ($uri) {
      return format_string('!hash *!file', array(
        '!hash' => sha1_file($uri),
        '!file' => drupal_basename($uri),
      ));
    };

    // XXX: Swap out Drupal's base URL, so any URLs we might generate should
    // be accessible.
    $old_base = $GLOBALS['base_url'];
    $GLOBALS['base_url'] = $info['accessed_base_url'];
    $urls = array_map('file_create_url', $uris);
    $meta_urls = array_map('file_create_url', array(
      'file_list' => file_unmanaged_save_data($array_to_string($urls), "$directory/files.txt"),
      'md5' => file_unmanaged_save_data($array_to_string(array_map($map_uri_to_md5line, $uris)), "$directory/md5.txt"),
      'sha1' => file_unmanaged_save_data($array_to_string(array_map($map_uri_to_sha1line, $uris)), "$directory/sha1.txt"),
    ));

    rules_invoke_event('islandora_zip_download_zip_generated', $info, $stats, $urls, $meta_urls, $size_constrained);
    $GLOBALS['old_base_url'] = $old_base;
  }
}

class IslandoraZipDownloadZipIterator implements RecursiveIterator {
  protected $exclude;
  protected $offset = 0;
  protected $start = array();
  protected $total = 0;
  protected $originalExclusions = array();
  protected $parent = NULL;

  /**
   * Constructor.
   *
   * @param array $start
   *   An array of PIDs from which to start iteration.
   * @param array $exclude
   *   An array of PIDs to exclude from iteration. Descendents will also be
   *   excluded (unless they can be accessed via another ancestor entailed from
   *   $start).
   * @param IslandoraZipDonwloadZipIterator|NULL $parent
   *   Either the parent iterator (to help build out the full directory path),
   *   or NULL if we are the initial/root iterator.
   */
  public function __construct($start, &$exclude = array(), $parent = NULL) {
    $this->start = array_values($start);
    $this->total = count($this->start);
    $this->backend = 'solr';
    $this->parent = $parent;
    if ($parent === NULL) {
      $this->originalExclusions = $exclude;
      $this->exclude = $exclude;
    }
    else {
      $this->exclude =& $exclude;
    }
  }

  /**
   * Helper; build out the base query processor.
   *
   * @return IslandoraSolrQueryProcessor
   *   A populated query processor to find the children of the current object.
   */
  protected function getChildrenQuery() {
    $qp = new IslandoraSolrQueryProcessor();
    $qp->buildQuery('*:*');
    $fields = array(
      variable_get('islandora_solr_member_of_field', 'RELS_EXT_isMemberOf_uri_ms'),
      variable_get('islandora_solr_member_of_collection_field', 'RELS_EXT_isMemberOfCollection_uri_ms'),
      variable_get('islandora_compound_object_relationship', 'isConstituentOf'),
    );
    $map_field_in = function($field) {
      return format_string('!field:"info:fedora/!pid"', array(
        '!field' => $field,
      ));
    };
    $qp->solrParams['fq'] = array(
      format_string(implode(' OR ', array_map($map_field_in, $fields)), array(
        '!pid' => $this->current(),
      )),
    );
    return $qp;
  }

  /**
   * Build array of path components to the current object.
   *
   * @return array
   *   An array of path components, each consisting of the object's label and
   *   PID.
   */
  protected function getCurrentNameComponents() {
    $components = $this->parent === NULL ?
        array() :
        $this->parent->getCurrentNameComponents();

    $object = islandora_object_load($this->current());

    if ($this->parent === NULL && !in_array('islandora:collectionCModel', $object->models)) {
      module_load_include('inc', 'islandora_basic_collection', 'includes/utilities');
      $parents = islandora_basic_collection_get_parent_pids($object);
      $parent_id = reset($parents);
      if ($parent_id && $parent = islandora_object_load($parent_id)) {
        $components[] = t('!label (!pid)', array(
          '!label' => str_replace('/', '__', $parent->label),
          '!pid' => $parent->id,
        ));
      }
    }

    $components[] = t('!label (!pid)', array(
      '!label' => $object->label,
      '!pid' => $object->id,
    ));

    return $components;
  }

  /**
   * Get a path-like name for the current file.
   *
   * @return string
   *   A path, in which each component is the label and PID of the object.
   */
  public function getCurrentName() {
    return implode('/', $this->getCurrentNameComponents());
  }

  /**
   * Inherits.
   */
  public function hasChildren() {
    // Test if $this->current() has children.
    if ($this->backend == 'solr') {
      $qp = $this->getChildrenQuery();
      $qp->solrLimit = 0;
      $qp->executeQuery();
      return $qp->islandoraSolrResult['response']['numFound'] > 0;
    }
  }

  /**
   * Inherits.
   */
  protected function lowerGetChildren() {
    if ($this->backend == 'solr') {
      $qp = $this->getChildrenQuery();
      // Arbitrarily large.
      $qp->solrLimit = 1000000;
      $qp->solrParams['fl'] = 'PID';
      $qp->solrParams['facet'] = 'false';
      $qp->executeQuery();
      $map_to_pids = function ($doc) {
        return $doc['PID'];
      };
      return array_map($map_to_pids, $qp->islandoraSolrResult['response']['objects']);
    }
  }

  /**
   * Inherits.
   */
  public function getChildren() {
    return new static($this->lowerGetChildren(), $this->exclude, $this);
  }

  /**
   * Inherits.
   */
  public function rewind() {
    $this->offset = 0;
    $this->exclude = $this->originalExclusions;
  }

  /**
   * Inherits.
   */
  public function current() {
    return $this->start[$this->offset];
  }

  /**
   * Inherits.
   */
  public function key() {
    return $this->current();
  }

  /**
   * Inherits.
   */
  public function valid() {
    return $this->offset < $this->total && !in_array($this->current(), $this->exclude);
  }

  /**
   * Inherits.
   */
  public function next() {
    $this->exclude[] = $this->current();
    while (!$this->valid() && $this->offset < $this->total) {
      $this->offset += 1;
    }
  }
}
