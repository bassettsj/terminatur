<?php

 /**
 * @file
 *  files.inc: Helper functions to download files
 */

/**
 * Gets the DB either by pipe or download adn then imports into mysql
 *
 * Validates and builds settings files it applicable.
 */
function _terminatur_files_rsync($site, $destination) {
  // Build files directories as appropriate
  // @todo some sort of error handling needs to happen here
  // we arent doing it now because this is fucking up box builds.
  _terminatur_files_set_up_dirs($site, $destination);
  drush_shell_exec("rsync " . $site['terminatur']['files']['rsync']['options'] . " " . $site['terminatur']['files']['rsync']['ssh'] . " " . $site['terminatur']['files']['rsync']['excludes'] . " " . $site['terminatur']['files']['rsync']['remote-files'] . " " . _terminatur_files_download_location($site, $destination) . TERMINATUR_DEFAULT_DEFAULT_DIR);
}

/**
 * Downloads a dump of the Pantheon database
 */
function _terminatur_files_download($site, $destination) {
  // Get the download link data
  if (!$site['terminatur']['files']['download-data'] = _terminatur_get_download_link($site, 'files')) {
    drush_log(dt("You have no backups. Try making a backup first."), 'warning');
    return FALSE;
  }

  // Build files directories as appropriate
  _terminatur_files_set_up_dirs($site, $destination);

  // Parse download data and begin the download
  $result = $site['terminatur']['files']['download-data'];
  $data = json_decode($result['json']);
  $filename = strstr(basename($data->url), '?', '_');
  $dir = strstr($filename, '.', true);
  $cache_duration = 86400*365;
  $path = _drush_download_file($data->url, TERMINATUR_TMP_DIR . $filename, $cache_duration);
  if ($path || drush_get_context('DRUSH_SIMULATE')) {
    drush_tarball_extract($path);
    drush_shell_exec("rm -rf " . _terminatur_files_download_location($site, $destination) . TERMINATUR_DEFAULT_DEFAULT_DIR . TERMINATUR_DEFAULT_FILES_DIR);
    rename(TERMINATUR_TMP_DIR . "files_" . $site['env'], _terminatur_files_download_location($site, $destination) . TERMINATUR_DEFAULT_DEFAULT_DIR . TERMINATUR_DEFAULT_FILES_DIR);
  }
  else {
    return drush_set_error('TERMINATUR_ERROR', 'Download failed.');
  }
}

/**
 * Gets the location to download the files
 */
function _terminatur_files_download_location($site, $destination) {
  if (is_dir($destination . $site['machine-name']) && is_writable($destination . $site['machine-name'])) {
    return $destination . $site['machine-name'];
  }
  else {
    return getenv('HOME') . DIRECTORY_SEPARATOR . $site['machine-name'];
  }
}

/**
 * Gets the location to download the files
 */
function _terminatur_files_set_up_dirs($site, $destination) {
  // Check files directory
  if (!is_dir(_terminatur_files_download_location($site, $destination) . TERMINATUR_DEFAULT_DEFAULT_DIR . TERMINATUR_DEFAULT_FILES_DIR)) {
    mkdir(_terminatur_files_download_location($site, $destination) . TERMINATUR_DEFAULT_DEFAULT_DIR . TERMINATUR_DEFAULT_FILES_DIR, 0777, TRUE);
  }
  if (!is_writable(_terminatur_files_download_location($site, $destination) . TERMINATUR_DEFAULT_DEFAULT_DIR . TERMINATUR_DEFAULT_FILES_DIR)) {
    chmod(_terminatur_files_download_location($site, $destination) . TERMINATUR_DEFAULT_DEFAULT_DIR . TERMINATUR_DEFAULT_FILES_DIR, 0777);
  }
  if (!is_dir(_terminatur_files_download_location($site, $destination) . TERMINATUR_DEFAULT_DEFAULT_DIR . TERMINATUR_DEFAULT_FILES_PRIVATE_DIR)) {
    mkdir(_terminatur_files_download_location($site, $destination) . TERMINATUR_DEFAULT_DEFAULT_DIR . TERMINATUR_DEFAULT_FILES_PRIVATE_DIR, 0777, TRUE);
  }
  if (!is_writable(_terminatur_files_download_location($site, $destination) . TERMINATUR_DEFAULT_DEFAULT_DIR . TERMINATUR_DEFAULT_FILES_PRIVATE_DIR)) {
    chmod(_terminatur_files_download_location($site, $destination) . TERMINATUR_DEFAULT_DEFAULT_DIR . TERMINATUR_DEFAULT_FILES_PRIVATE_DIR, 0777);
  }
}

/**
 * Removes the files
 */
function _terminatur_files_remove($site, $destination) {
  // Remove Code and check for a site name and destination before we kill everything by accident
  if (isset($site) && isset($destination)) {
    $files_dir = _terminatur_files_download_location($site, $destination) . TERMINATUR_DEFAULT_DEFAULT_DIR . TERMINATUR_DEFAULT_FILES_DIR;
    // Remove the files dir and everything in it.
    // Need to run shell command because PHP isnt great at complete destruction
    if (!drush_shell_exec("rm -rf " . $files_dir)) {
      return drush_set_error('TERMINATUR_ERROR', 'Could not remove files. Please remove manually.');
    }
  }
  return TRUE;
}

