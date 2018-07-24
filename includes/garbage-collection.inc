<?php

/**
 * @file
 * Garbage collection and related functions.
 */

/**
 * Perform the collection.
 */
function islandora_zip_download_collect_garbage() {
  $now = time();
  $q = db_select('islandora_zip_download_tracking', 't')
    ->fields('t', array())
    ->condition(db_or()->condition('expiry', $now, '<')->isNull('expiry'));
  $get_max_contained_age = function ($path) {
    $get_max_time = function ($uri) {
      static $max = NULL;
      if ($uri === NULL) {
        return $max;
      }
      elseif ($max === NULL) {
        $max = filemtime($uri);
      }
      else {
        $max = max($max, filemtime($uri));
      }
    };
    file_scan_directory($path, '/.*/', array(
      'callback' => $get_max_time,
    ));
    return $get_max_time(NULL);
  };
  $ttl = variable_get('islandora_zip_download_ttl', 48) * 3600;
  foreach ($q->execute() as $result) {
    if ($result->expiry === NULL) {
      $max_contained_age = $get_max_contained_age($result->path);
      $offset_max_age = $max_contained_age + $ttl;
      if ($now > $offset_max_age) {
        // The max last modified date is at least the ttl ago, give it another
        // ttl.
        db_update('islandora_zip_download_tracking')
          ->fields(array('expiry' => $offset_max_age + $ttl))
          ->condition('path', $result->path)
          ->execute();
      }
      continue;
    }
    if (file_unmanaged_delete_recursive($result->path)) {
      db_delete('islandora_zip_download_tracking')
        ->condition('path', $result->path)
        ->execute();
    }
  }
}
