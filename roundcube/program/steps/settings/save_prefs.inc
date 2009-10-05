<?php

/*
 +-----------------------------------------------------------------------+
 | program/steps/settings/save_prefs.inc                                 |
 |                                                                       |
 | This file is part of the RoundCube Webmail client                     |
 | Copyright (C) 2005-2009, RoundCube Dev. - Switzerland                 |
 | Licensed under the GNU GPL                                            |
 |                                                                       |
 | PURPOSE:                                                              |
 |   Save user preferences to DB and to the current session              |
 |                                                                       |
 +-----------------------------------------------------------------------+
 | Author: Thomas Bruederli <roundcube@gmail.com>                        |
 +-----------------------------------------------------------------------+

 $Id: save_prefs.inc 2873 2009-08-27 06:18:54Z thomasb $

*/

$CURR_SECTION = get_input_value('_section', RCUBE_INPUT_POST);

$a_user_prefs = array();

// set options for specified section
switch ($CURR_SECTION)
{
  case 'general':
    $a_user_prefs = array(
      'language'     => isset($_POST['_language']) ? get_input_value('_language', RCUBE_INPUT_POST) : $CONFIG['language'],
      'timezone'     => isset($_POST['_timezone']) ? (is_numeric($_POST['_timezone']) ? floatval($_POST['_timezone']) : get_input_value('_timezone', RCUBE_INPUT_POST)) : $CONFIG['timezone'],
      'dst_active'   => isset($_POST['_dst_active']) ? TRUE : FALSE,
      'pagesize'     => is_numeric($_POST['_pagesize']) ? max(2, intval($_POST['_pagesize'])) : $CONFIG['pagesize'],
      'prettydate'   => isset($_POST['_pretty_date']) ? TRUE : FALSE,
      'skin' 	     => isset($_POST['_skin']) ? get_input_value('_skin', RCUBE_INPUT_POST) : $CONFIG['skin'],
    );

  break;
  case 'mailbox':
    $a_user_prefs = array(
      'focus_on_new_message' => isset($_POST['_focus_on_new_message']) ? TRUE : FALSE,
      'preview_pane' 	     => isset($_POST['_preview_pane']) ? TRUE : FALSE,
      'mdn_requests'         => isset($_POST['_mdn_requests']) ? intval($_POST['_mdn_requests']) : 0,
      'keep_alive'           => isset($_POST['_keep_alive']) ? intval($_POST['_keep_alive'])*60 : $CONFIG['keep_alive'],
      'check_all_folders'    => isset($_POST['_check_all_folders']) ? TRUE : FALSE,
    );

  break;
  case 'mailview':
    $a_user_prefs = array(
      'prefer_html'     => isset($_POST['_prefer_html']) ? TRUE : FALSE,
      'inline_images'   => isset($_POST['_inline_images']) ? TRUE : FALSE,
      'show_images'     => isset($_POST['_show_images']) ? intval($_POST['_show_images']) : 0,
      'display_next'    => isset($_POST['_display_next']) ? TRUE : FALSE,
    );

  break;
  case 'compose':
    $a_user_prefs = array(
      'htmleditor'         => isset($_POST['_htmleditor']) ? TRUE : FALSE,
      'draft_autosave'     => isset($_POST['_draft_autosave']) ? intval($_POST['_draft_autosave']) : 0,
      'mime_param_folding' => isset($_POST['_mime_param_folding']) ? intval($_POST['_mime_param_folding']) : 0,
    );

  break;
  case 'server':
    $a_user_prefs = array(
      'read_when_deleted' => isset($_POST['_read_when_deleted']) ? TRUE : FALSE,
      'skip_deleted'      => isset($_POST['_skip_deleted']) ? TRUE : FALSE,
      'flag_for_deletion' => isset($_POST['_flag_for_deletion']) ? TRUE : FALSE,
      'logout_purge'      => isset($_POST['_logout_purge']) ? TRUE : FALSE,
      'logout_expunge'    => isset($_POST['_logout_expunge']) ? TRUE : FALSE,
    );

  break;
  case 'folders':
    $a_user_prefs = array(
      'drafts_mbox' => get_input_value('_drafts_mbox', RCUBE_INPUT_POST),
      'sent_mbox'   => get_input_value('_sent_mbox', RCUBE_INPUT_POST),
      'junk_mbox'   => get_input_value('_junk_mbox', RCUBE_INPUT_POST),
      'trash_mbox'  => get_input_value('_trash_mbox', RCUBE_INPUT_POST),
    );

  break;
}


$data = rcmail::get_instance()->plugins->exec_hook('save_preferences',
  array('prefs' => $a_user_prefs, 'section' => $CURR_SECTION));

$a_user_prefs = $data['prefs'];

// don't override these parameters
foreach ((array)$CONFIG['dont_override'] as $p)
  $a_user_prefs[$p] = $CONFIG[$p];


// verify some options
switch ($CURR_SECTION)
{
  case 'general':

    // switch UI language
    if (isset($_POST['_language']) && $a_user_prefs['language'] != $_SESSION['language']) {
      $RCMAIL->load_language($a_user_prefs['language']);
      $OUTPUT->command('reload', 500);
    }

    // switch skin
    $OUTPUT->set_skin($a_user_prefs['skin']);

    // force min size
    if ($a_user_prefs['pagesize'] < 1)
      $a_user_prefs['pagesize'] = 10;

    if (isset($CONFIG['max_pagesize']) && ($a_user_prefs['pagesize'] > $CONFIG['max_pagesize']))
      $a_user_prefs['pagesize'] = (int) $CONFIG['max_pagesize'];

  break;
  case 'mailbox':

    // force keep_alive
    if (isset($a_user_prefs['keep_alive'])) {
      $a_user_prefs['keep_alive'] = max(60, $CONFIG['min_keep_alive'], $a_user_prefs['keep_alive']);
      if (!empty($CONFIG['session_lifetime']))
        $a_user_prefs['keep_alive'] = min($CONFIG['session_lifetime']*60, $a_user_prefs['keep_alive']);
    }

  break;
  case 'folders':

    // special handling for 'default_imap_folders'
    if (in_array('default_imap_folders', (array)$CONFIG['dont_override'])) {
      foreach (array('drafts_mbox','sent_mbox','junk_mbox','trash_mbox') as $p)
        $a_user_prefs[$p] = $CONFIG[$p];
    } else {
      $a_user_prefs['default_imap_folders'] = array('INBOX');
      foreach (array('drafts_mbox','sent_mbox','junk_mbox','trash_mbox') as $p) {
        if ($a_user_prefs[$p])
          $a_user_prefs['default_imap_folders'][] = $a_user_prefs[$p];
      }
    }
  
  break;
}

if ($USER->save_prefs($a_user_prefs))
  $OUTPUT->show_message('successfullysaved', 'confirmation');

// display the form again
rcmail_overwrite_action('edit-prefs');

?>