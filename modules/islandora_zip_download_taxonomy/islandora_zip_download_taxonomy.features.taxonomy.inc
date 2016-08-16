<?php
/**
 * @file
 * islandora_zip_download_taxonomy.features.taxonomy.inc
 */

/**
 * Implements hook_taxonomy_default_vocabularies().
 */
function islandora_zip_download_taxonomy_taxonomy_default_vocabularies() {
  return array(
    'islandora_zip_download_mime_type_labels' => array(
      'name' => 'Islandora ZIP Download MIME Type labels',
      'machine_name' => 'islandora_zip_download_mime_type_labels',
      'description' => 'Stores a mapping of MIME types to human readable labels',
      'hierarchy' => 0,
      'module' => 'taxonomy',
      'weight' => 0,
      'rdf_mapping' => array(
        'rdftype' => array(
          0 => 'skos:ConceptScheme',
        ),
        'name' => array(
          'predicates' => array(
            0 => 'dc:title',
          ),
        ),
        'description' => array(
          'predicates' => array(
            0 => 'rdfs:comment',
          ),
        ),
      ),
    ),
  );
}
