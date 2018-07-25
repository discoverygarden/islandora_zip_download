# Islandora ZIP Download

## Introduction

Provides an exporter for the Islandora Bookmark module which allows the user to export a hierarchial structure of collections, objects and their children in ZIP format.

## Requirements

This module requires the following modules/libraries:

* [Islandora](https://github.com/islandora/islandora)
* [Tuque](https://github.com/islandora/tuque)
* [Islandora Solr](https://github.com/islandora/islandora_solr_search)
* [Islandora Bookmark](https://github.com/islandora/islandora_bookmark)
* [Islandora Job](https://github.com/discoverygarden/islandora_job)
* [UUID](https://www.drupal.org/project/uuid)
* [Rules](https://www.drupal.org/project/rules)

It is highly recommended that the sub-module
islandora_zip_download_rules_email be enabled or a similar rule be created.

## Installation

Install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.

In order to have the Gearman workers listen for this job, you will need to stop & start the workers:

```
service gearman-workers stop
service gearman-workers start
```

Depending on the final configuration, it may be necessary to install at least [version 1.11.0 of the "zip" extension](https://pecl.php.net/package-changelog.php?package=zip), in order to obtain ZIP64 support, to support large file sizes.

## Configuration

Configure general bookmark settings in Administration » Islandora » Islandora Utility Modules » Islandora ZIP Download (admin/islandora/tools/islandora-zip-download).

NOTE: A field indexing all MIME types of a datastream is required if MIME type filtering is to be available through the UI.

## Troubleshooting/Issues

Having problems or solved a problem? Contact [discoverygarden](http://support.discoverygarden.ca).

## Maintainers/Sponsors

Current maintainers:

* [discoverygarden](http://www.discoverygarden.ca)

Sponsors:

* [Max Planck Institute for Psycholinguistics](http://www.mpi.nl/)

## Development

If you would like to contribute to this module, please check out our helpful
[Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers)
info, [Developers](http://islandora.ca/developers) section on Islandora.ca and
contact [discoverygarden](http://support.discoverygarden.ca).

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
