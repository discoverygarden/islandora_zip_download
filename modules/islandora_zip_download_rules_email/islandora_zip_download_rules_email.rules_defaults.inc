<?php
/**
 * @file
 * islandora_zip_download_rules_email.rules_defaults.inc
 */

/**
 * Implements hook_default_rules_configuration().
 */
function islandora_zip_download_rules_email_default_rules_configuration() {
  $items = array();
  $items['rules_islandora_zip_download_complete_constrained'] = entity_import('rules_config', '{ "rules_islandora_zip_download_complete_constrained" : {
      "LABEL" : "islandora_zip_download_complete_constrained",
      "PLUGIN" : "reaction rule",
      "OWNER" : "rules",
      "REQUIRES" : [ "rules", "islandora_zip_download" ],
      "ON" : { "islandora_zip_download_zip_generated" : [] },
      "IF" : [ { "data_is" : { "data" : [ "constrained" ], "value" : "1" } } ],
      "DO" : [
        { "variable_add" : {
            "USING" : { "type" : "text" },
            "PROVIDE" : { "variable_added" : { "files" : "Files" } }
          }
        },
        { "LOOP" : {
            "USING" : { "list" : [ "paths" ] },
            "ITEM" : { "file" : "File" },
            "DO" : [
              { "data_set" : { "data" : [ "files" ], "value" : "[files:value]\r\n[file:value]" } }
            ]
          }
        },
        { "mail" : {
            "to" : [ "site:current-user:mail" ],
            "subject" : "Your ZIP file is ready for download",
            "message" : "Your ZIP file is ready for download. Added [stats:count] datastreams at a total size of [stats:size:source:human_readable] compressed to [stats:size:compressed:human_readable]. Size limit reached, the resulting ZIP does not contain all of the resources selected. \r\n\r\nPlease download it via the link below within the next [expiry:value] hours, as it will be deleted afterwards. \r\n\r\n[files:value]\r\n\r\nTo verify the integrity of the archive there is both and MD5 and SHA1 of the ZIP and any of its available parts.\r\n\r\nMD5: [meta:md5]\r\nSHA1: [meta:sha1]\r\nFile list: [meta:file-list]",
            "from" : "[site:name]",
            "language" : [ "" ]
          }
        }
      ]
    }
  }');
  $items['rules_islandora_zip_download_complete_full'] = entity_import('rules_config', '{ "rules_islandora_zip_download_complete_full" : {
      "LABEL" : "islandora_zip_download_complete_full",
      "PLUGIN" : "reaction rule",
      "OWNER" : "rules",
      "REQUIRES" : [ "rules", "islandora_zip_download" ],
      "ON" : { "islandora_zip_download_zip_generated" : [] },
      "IF" : [ { "data_is" : { "data" : [ "constrained" ], "value" : "0" } } ],
      "DO" : [
        { "variable_add" : {
            "USING" : { "type" : "text" },
            "PROVIDE" : { "variable_added" : { "files" : "Files" } }
          }
        },
        { "LOOP" : {
            "USING" : { "list" : [ "paths" ] },
            "ITEM" : { "file" : "File" },
            "DO" : [
              { "data_set" : { "data" : [ "files" ], "value" : "[files:value]\r\n[file:value]" } }
            ]
          }
        },
        { "mail" : {
            "to" : [ "site:current-user:mail" ],
            "subject" : "Your ZIP file is ready for download",
            "message" : "Your ZIP file is ready for download. Added [stats:count] datastreams at a total size of [stats:size:source:human_readable] compressed to [stats:size:compressed:human_readable].\r\n\r\nPlease download it via the link below within the next [expiry:value] hours, as it will be deleted afterwards. \r\n\r\n[files:value]\r\n\r\nTo verify the integrity of the archive there is both an MD5 and SHA1 of the ZIP and any of its available parts.\r\n\r\nMD5: [meta:md5]\r\nSHA1: [meta:sha1]\r\nFile list: [meta:file-list]",
            "from" : "[site:name]",
            "language" : [ "" ]
          }
        }
      ]
    }
  }');
  $items['rules_islandora_zip_download_incomplete_constrained'] = entity_import('rules_config', '{ "rules_islandora_zip_download_incomplete_constrained" : {
      "LABEL" : "islandora_zip_download_incomplete_constrained",
      "PLUGIN" : "reaction rule",
      "OWNER" : "rules",
      "REQUIRES" : [ "rules", "islandora_zip_download" ],
      "ON" : { "islandora_zip_download_zip_empty" : [] },
      "IF" : [ { "data_is" : { "data" : [ "constrained" ], "value" : "1" } } ],
      "DO" : [
        { "mail" : {
            "to" : [ "site:current-user:mail" ],
            "subject" : "ZIP file creation failed",
            "message" : "The resulting selection yielded no files to be downloaded as the size limit was reached.",
            "from" : "[site:name]",
            "language" : [ "" ]
          }
        }
      ]
    }
  }');
  $items['rules_islandora_zip_download_incomplete_empty'] = entity_import('rules_config', '{ "rules_islandora_zip_download_incomplete_empty" : {
      "LABEL" : "islandora_zip_download_incomplete_empty",
      "PLUGIN" : "reaction rule",
      "OWNER" : "rules",
      "REQUIRES" : [ "rules", "islandora_zip_download" ],
      "ON" : { "islandora_zip_download_zip_empty" : [] },
      "IF" : [ { "data_is" : { "data" : [ "constrained" ], "value" : "0" } } ],
      "DO" : [
        { "mail" : {
            "to" : [ "site:current-user:mail" ],
            "subject" : "ZIP file creation failed",
            "message" : "The resulting selection yielded no files to be downloaded.",
            "from" : "[site:name]",
            "language" : [ "" ]
          }
        }
      ]
    }
  }');
  return $items;
}
