<?php

/**
 * @file
 * Holds utility functions for the Islandora ZIP Download module.
 */

/**
 * Retrieves a listing of MIME types and labels for use in the export UI.
 *
 * @return bool|array
 *   An associative array where the key is the value in the select and the value
 *   is the human readable label or FALSE if no MIMEs were found.
 */
function islandora_zip_download_mimetypes_to_export() {
  module_load_include('inc', 'islandora_zip_download_taxonomy', 'includes/utilities');
  $mimes = islandora_zip_download_mimetypes_in_use();
  if ($mimes) {
    // Grab any human-readable variables and replace them as the values.
    $human_readable = islandora_zip_download_taxonomy_human_readable_mimetypes();
    $replaced_array = array_replace($mimes, $human_readable);
    asort($replaced_array);
    return $replaced_array;
  }
  return FALSE;
}

/**
 * Helper function that constructs and executes a Solr query to retrieve MIMEs.
 *
 * @param bool|string $facet_prefix
 *   FALSE if not being used, or a string denoting what to set the facet.prefix
 *   value to.
 *
 * @return bool|array
 *   FALSE if no results found, an associative array where the keys and values
 *   are the MIME types.
 */
function islandora_zip_download_mimetypes_in_use($facet_prefix = FALSE) {
  $facet_field = variable_get('islandora_zip_download_mimetype_solr_field', 'fedora_datastreams_mimetypes_ms');
  $qp = new IslandoraSolrQueryProcessor();
  $qp->buildQuery('*:*');
  $qp->solrParams['fl'] = 'PID';
  $qp->solrLimit = 1;
  $qp->solrParams['facet'] = 'true';
  $qp->solrParams['facet.field'] = $facet_field;
  $qp->solrParams['facet.mincount'] = 0;
  if ($facet_prefix) {
    $qp->solrParams['facet.prefix'] = $facet_prefix;
  }
  $qp->executeQuery(FALSE);
  if (!empty($qp->islandoraSolrResult['facet_counts']['facet_fields'][$facet_field])) {
    return drupal_map_assoc(array_keys(array_filter($qp->islandoraSolrResult['facet_counts']['facet_fields'][$facet_field])));
  }
  return FALSE;
}
