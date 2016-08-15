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
  $facet_field = variable_get('islandora_zip_download_mimetype_solr_field', 'fedora_datastreams_mimetypes_ms');
  $qp = new IslandoraSolrQueryProcessor();
  $qp->buildQuery('*:*');
  $qp->solrParams['fl'] = 'PID';
  $qp->solrLimit = 1;
  $qp->solrParams['facet'] = 'true';
  $qp->solrParams['facet.field'] = $facet_field;
  $qp->solrParams['facet.mincount'] = 0;
  $qp->executeQuery(FALSE);
  if (!empty($qp->islandoraSolrResult['facet_counts']['facet_fields'][$facet_field])) {
    $mimes = drupal_map_assoc(array_keys(array_filter($qp->islandoraSolrResult['facet_counts']['facet_fields'][$facet_field])));
    // Grab any human-readable variables and replace them as the values.
    $human_readable = islandora_zip_download_taxonomy_human_readable_mimetypes();
    return array_replace($mimes, $human_readable);
  }
  return FALSE;
}
