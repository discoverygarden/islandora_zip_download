<?php
/**
 * @file
 * Utility functions for the ZIP Download Taxonomy module.
 */

/**
 * Retrieves all the taxonomy terms for human readable labels for MIME types.
 *
 * @param bool $table_output
 *   Whether this is being used for a UI table output.
 *
 * @return array
 *   An associate array listing human readable labels for MIME types where the
 *   key is the MIME type and the value is the label.
 */
function islandora_zip_download_taxonomy_human_readable_mimetypes($table_output = FALSE) {
  $vocabulary = taxonomy_vocabulary_machine_name_load('islandora_zip_download_mime_type_labels');
  $terms = entity_load('taxonomy_term', FALSE, array('vid' => $vocabulary->vid));
  $human_readable = array();
  foreach ($terms as $term) {
    if ($table_output) {
      $human_readable[] = array($term->name, $term->description);
    }
    else {
      $human_readable[$term->name] = $term->description;
    }
  }
  return $human_readable;
}
