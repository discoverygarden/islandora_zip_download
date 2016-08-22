<?php
/**
 * @file
 * Administration form for Islandora ZIP Download.
 */

/**
 * Defines the admin settings form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_zip_download_admin_form($form, $form_state) {
  module_load_include('inc', 'islandora_zip_download_taxonomy', 'includes/utilities');
  $human_readable_mimes = islandora_zip_download_taxonomy_human_readable_mimetypes(TRUE);
  $form['config'] = array(
    '#type' => 'vertical_tabs',
  );
  $form['config']['general_configuration'] = array(
    '#type' => 'fieldset',
    '#title' => t('General Configuration'),
    'islandora_zip_download_mimetype_solr_field' => array(
      '#type' => 'textfield',
      '#title' => t('MIME type Solr field'),
      '#required' => TRUE,
      '#default_value' => variable_get('islandora_zip_download_mimetype_solr_field', 'fedora_datastreams_mimetypes_ms'),
    ),
    'islandora_zip_download_pids_to_exclude' => array(
      '#type' => 'textarea',
      '#title' => t('PIDs to exclude (comma separated)'),
      '#default_value' => variable_get('islandora_zip_download_pids_to_exclude', ''),
    ),
    'islandora_zip_download_dsids_to_exclude' => array(
      '#type' => 'textarea',
      '#title' => t('Datastream IDs to exclude (comma separated)'),
      '#default_value' => variable_get('islandora_zip_download_dsids_to_exclude', ''),
    ),
    'islandora_zip_download_mimes_to_exclude' => array(
      '#type' => 'textarea',
      '#title' => t('MIME Types to exclude (comma separated)'),
      '#default_value' => variable_get('islandora_zip_download_mimes_to_exclude', ''),
    ),
  );
  $size_defaults = variable_get('islandora_zip_download_size_limits', array(
    'scale' => 30,
    'source_limit' => 0,
    'export_limit' => 0,
    'split_size' => 0,
    'split_executable' => 'split',
  ));
  $form['config']['islandora_zip_download_size_limits'] = array(
    '#type' => 'fieldset',
    '#title' => t('Size Limits'),
    '#tree' => TRUE,
    'scale' => array(
      '#type' => 'radios',
      '#title' => t('Units'),
      '#description' => t('The units given for the following measurements.'),
      '#default_value'=> $size_defaults['scale'],
      '#options' => array(
        0 => t('bytes'),
        10 => t('kibibytes'),
        20 => t('mebibytes'),
        30 => t('gibibytes'),
      ),
    ),
    'source_limit' => array(
      '#type' => 'textfield',
      '#title' => t('Source limit'),
      '#description' => t('The size of the source threshold size. 0 or negative to disable.'),
      '#element_validate' => array(
        'element_validate_number',
      ),
      '#required' => TRUE,
      '#default_value' => $size_defaults['source_limit'],
    ),
    'export_limit' => array(
      '#type' => 'textfield',
      '#title' => t('Export limit'),
      '#description' => t('The size of the archive threshold size. 0 or negative to disable.'),
      '#element_validate' => array(
        'element_validate_number',
      ),
      '#required' => TRUE,
      '#default_value' => $size_defaults['export_limit'],
    ),
    'split_size' => array(
      '#type' => 'textfield',
      '#title' => t('Split size'),
      '#description' => t('The archive will be split into files no larger than the size specified. 0 or negative to disable.'),
      '#element_validate' => array(
        'element_validate_number',
      ),
      '#required' => TRUE,
      '#default_value' => $size_defaults['split_size'],
    ),
    'split_executable' => array(
      '#type' => 'textfield',
      '#title' => t('Split executable'),
      '#description' => t('Path to the split executable'),
      '#required' => TRUE,
      '#default_value' => $size_defaults['split_executable'],
    ),
  );
  $form['config']['file_type_labels'] = array(
    '#type' => 'fieldset',
    '#title' => t('File Type Labels'),
    'table' => array(
      '#theme' => 'table',
      '#header' => array(t('MIME Type'), t('Label')),
      '#rows' => $human_readable_mimes,
      '#caption' => t('Manage human-readable MIME types through <a href="@url">taxonomy.</a>', array(
        '@url' => url('admin/structure/taxonomy/islandora_zip_download_mime_type_labels'),
      )),
    ),
  );
  $form['#submit'][] = 'islandora_zip_download_admin_form_submit';
  return system_settings_form($form);
}
