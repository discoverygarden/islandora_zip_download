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
    'variables' => array(
      'params' => array(
        'label' => t('Generation params'),
        'type' => 'list<list<text>>',
      ),
      'stats' => array(
        'label' => t('Generation stats'),
        'type' => 'list<list<text>>',
      ),
      'limits' => array(
        'label' => t('Generation limits'),
        'type' => 'list<int>',
      ),
      'paths' => array(
        'label' => t('ZIP files'),
        'type' => 'list<uri>',
      ),
    ),
  );
  $event['islandora_zip_download_zip_empty'] = array(
    'label' => t('ZIP Generation resulted in empty archive'),
    'variables' => array(
      'params' => array(
        'label' => t('Generation params'),
        'type' => 'list<list<text>>',
      ),
    ),
  );

  return $event;
}
