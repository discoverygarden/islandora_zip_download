<?php

function islandora_zip_download_generate_zip($info) {
  // Need to act as the given user, so as not to expose resources to which
  // the user should not have access.
  global $user;
  $user = user_load($info['user_id']);

  $zip_file = drupal_tempnam('temporary://', 'islandora_zip_download');
  $zip = new ZipArchive();
  $stats = array(
    'count' => 0,
    'size' => array(
      'source' => 0,
      'compressed' => 0,
    ),
  );

  $check_stats = function() use ($stats) {
    // TODO: Implement... Throw exceptions if we exceed any limits.
  };

  $write_file_to_zip = function ($actual_file, $destination_name = NULL) use ($zip, $zip_file, &$stats, $check_stats) {
    $zip->open($zip_file);
    $zip->addFile($actual_file, $destination_name);
    $zip->close();
    $stats['count'] += 1;
    $stats['size']['source'] += filesize($actual_file);
    $stats['size']['compressed'] = filesize($zip_file);
    $check_stats();
  };
  $datastream_filename = function (AbstractDatastream $datastream) {
    //XXX: Copy/pasta from islandora_view_datastream(), when download === TRUE.
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
  $write_datastream_to_zip = function (Iterator $iterator, $dir) use ($write_file_to_zip, $datastream_filename) {
    $datastream = $iterator->current();
    try {
      $tempfile = drupal_tempnam('temporary://', 'islandora_zip_download_ds');
      $datastream->getContent($tempfile);
      $filename = $datastream_filename($datastream);
      $write_file_to_zip($tempfile, "$dir/$filename");
    }
    finally {
      drupal_unlink($tempfile);
    }
  };
  $unfiltered = function (AbstractDatastream $datastream, $key, $iterator) use ($info) {
    $id = $datastream->id;
    $mime = $datastream->mimetype;

    return !in_array($id, $info['dsids']['exclude']) && (
        empty($info['mimes']['all']) || in_array($datastream->mimetype, $info['mimes']['filtered']))
  };
  $iterator = new RecursiveIteratorIterator(new IslandoraZipDownloadZipIterator($info['pids']['filtered'], $info['pids']['exclude']), RecursiveIteratorIterator::SELF_FIRST);
  foreach ($iterator as $pid) {
    $object = islandora_object_load($pid);

    iterator_apply(new CallbackFilterIterator($object, $unfiltered), $write_datastream_to_zip, array($iterator->getCurrentName()));
  }
}

class IslandoraZipDownloadZipIterator implements RecursiveIterator {
  protected $exclude;
  protected $offset = 0;
  protected $start = array();
  protected $total = 0;
  protected $original_exclusions = array();
  protected $parent = NULL;

  public function __construct($start, &$exclude = array(), $parent = NULL) {
    $this->start = $start;
    $this->total = count($start);
    $this->backend = 'solr';
    $this->parent = $parent;
    if ($parent === NULL) {
      $this->original_exclusions = $exclude;
      $this->exclude = $exclude;
    }
    else {
      $this->exclude =& $exclude;
    }
  }

  protected function getChildrenQuery() {
      $qp = new IslandoraSolrQueryProcessor();
      $qp->buildQuery('*:*');
      $base = array(
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
        ))
      );
      return $qp
  }

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
          '!label' => $parent->label,
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

  public function getCurrentName() {
    return implode('/', $this->getCurrentNameComponents());
  }

  public function hasChildren() {
    // Test if $this->current() has children.
    if ($this->backend == 'solr') {
      $qp = $this->getChildrenQuery()
      $qp->solrLimit = 0;
      $qp->executeQuery();
      return $qp->islandoraSolrResult['response']['numFound'] > 0;
    }
  }

  protected function _getChildren() {
    if ($this->backend == 'solr') {
      $qp = $this->getChildrenQuery()
      // Arbitrarily large.
      $qp->solrLimit = 1000000;
      $qp->solrParams['fl'] = 'PID';
      $qp->solrParams['facet'] = FALSE;
      $qp->executeQuery();
      $map_to_pids = function ($doc) {
        return $doc['PID'];
      };
      return array_map($map_to_pids, $qp->islandoraSolrResult['response']['objects']);
    }
  }

  public function getChildren() {
    // Return an instance referring to
    return new static($this->_getChildren(), $this->exclude, $this);
  }

  public function rewind() {
    $this->offset = 0;
    $this->exclude = $this->original_exclusions;
  }
  public function current() {
    return $this->start[$this->offset];
  }
  public function valid() {
    return $this->offset < $this->total && !in_array($this->current(), $this->exclude);
  }
  public function next() {
    $this->exclude[] = $this->current();
    while (!$this->valid() && $this->offset < $this->total) {
      $this->offset += 1;
    }
  }
}
