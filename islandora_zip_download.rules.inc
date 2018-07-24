<?php

/**
 * @file
 * Rules hook implementations.
 */

/**
 * Implements hook_rules_event_info().
 */
function islandora_zip_download_rules_event_info() {
  $event = array();

  $event['islandora_zip_download_zip_generated'] = array(
    'label' => t('ZIP Generation Complete'),
    'group' => t('Islandora ZIP Download'),
    'variables' => array(
      'stats' => array(
        'label' => t('Generation stats'),
        'description' => t('Datastream count and file size information.'),
        'type' => 'islandora_zip_download_generation_stats',
      ),
      'paths' => array(
        'label' => t('ZIP files'),
        'description' => t('All ZIP files (and reassembly helper scripts). A file containing the same is available as "file_list" on "meta".'),
        'type' => 'list<uri>',
      ),
      'meta' => array(
        'label' => t('Meta files'),
        'description' => t('URLs to files describing the files.'),
        'type' => 'islandora_zip_download_meta_files',
      ),
      'constrained' => array(
        'label' => t('Constrained?'),
        'description' => t('A boolean flag indicating if the generation of ZIP files was stopped prematurely due to hitting a size constraint (some data which was selected was not included).'),
        'type' => 'boolean',
      ),
      'expiry' => array(
        'label' => t('Expiry'),
        'type' => 'integer',
      ),
    ),
  );
  $event['islandora_zip_download_zip_empty'] = array(
    'label' => t('ZIP Generation resulted in empty archive'),
    'group' => t('Islandora ZIP Download'),
    'variables' => array(
      'params' => array(
        'label' => t('Generation params'),
        'type' => 'list<list<text>>',
      ),
      'constrained' => array(
        'label' => t('Constrained?'),
        'description' => t('A boolean flag indicating if the generation of ZIP files was stopped prematurely due to hitting a size constraint (some data which was selected was not included).'),
        'type' => 'boolean',
      ),
    ),
  );

  return $event;
}

/**
 * Implements hook_rules_data_info().
 */
function islandora_zip_download_rules_data_info() {
  $type = array();

  $file_size_properties = array(
    'human_readable' => array(
      'label' => t('Human-readable file size'),
      'type' => 'text',
      'getter callback' => 'islandora_zip_download_rules_file_size_to_human_readable',
      'computed' => TRUE,
    ),
  );
  $type['islandora_zip_download_generation_stats'] = array(
    'label' => t('Generation statistics'),
    'group' => t('Islandora ZIP Download'),
    'type' => 'struct',
    'wrap' => TRUE,
    'property info' => array(
      'count' => array(
        'label' => t('Datastream count'),
        'type' => 'integer',
      ),
      'size' => array(
        'label' => t('Size statistics'),
        'type' => 'struct',
        'property info' => array(
          'source' => array(
            'label' => t('Source size'),
            'type' => 'integer',
            'property info' => $file_size_properties,
          ),
          'compressed' => array(
            'label' => t('Compressed size'),
            'type' => 'integer',
            'property info' => $file_size_properties,
          ),
        ),
      ),
    ),
  );
  $type['islandora_zip_download_meta_files'] = array(
    'label' => t('Meta info'),
    'group' => t('Islandora ZIP Download'),
    'type' => 'list<uri>',
    'wrap' => TRUE,
    'property info' => array(
      'file_list' => array(
        'label' => t('File List'),
        'type' => 'uri',
      ),
      'md5' => array(
        'label' => t('MD5 sum file'),
        'type' => 'uri',
      ),
      'sha1' => array(
        'label' => t('SHA1 sum file'),
        'type' => 'uri',
      ),
    ),
  );

  return $type;
}
