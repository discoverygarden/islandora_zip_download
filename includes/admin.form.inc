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
    'mimetype_solr_field' => array(
      '#type' => 'textfield',
      '#title' => t('MIME type Solr field'),
      '#required' => TRUE,
      '#default_value' => variable_get('islandora_zip_download_mimetype_solr_field', 'fedora_datastreams_mimetypes_ms'),
    ),
    'pids_to_exclude' => array(
      '#type' => 'textarea',
      '#title' => t('PIDs to exclude (comma separated)'),
      '#default_value' => variable_get('islandora_zip_download_pids_to_exclude', ''),
    ),
    'dsids_to_exclude' => array(
      '#type' => 'textarea',
      '#title' => t('Datastream IDs to exclude (comma separated)'),
      '#default_value' => variable_get('islandora_zip_download_dsids_to_exclude', ''),
    ),
    'mimetypes_to_exclude' => array(
      '#type' => 'textarea',
      '#title' => t('MIME Types to exclude (comma separated)'),
      '#default_value' => variable_get('islandora_zip_download_mimes_to_exclude', ''),
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
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Submit handler for the general configuration form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_zip_download_admin_form_submit($form, $form_state) {
  variable_set('islandora_zip_download_mimetype_solr_field', $form_state['values']['mimetype_solr_field']);
  variable_set('islandora_zip_download_pids_to_exclude', $form_state['values']['pids_to_exclude']);
  variable_set('islandora_zip_download_dsids_to_exclude', $form_state['values']['dsids_to_exclude']);
  variable_set('islandora_zip_download_mimes_to_exclude', $form_state['values']['mimetypes_to_exclude']);
}
