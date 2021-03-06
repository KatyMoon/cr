<?php
// $Id: persistent_login.pages.inc,v 1.1.2.4 2009/04/26 03:53:33 markuspetrux Exp $

/**
 * @file
 * Implementation of Persistent Login forms.
 */

/**
 * Module settings form.
 */
function persistent_login_admin_settings() {
  $form = array();

  // @todo: Can we move this to hook_requirements() ?
  if (ini_get('session.cookie_lifetime') > 0) {
    drupal_set_message(t('Your site\'s <em>session.cookie_lifetime</em> PHP setting is %life.  When using Persistent Login, it should be 0 so that PHP sessions end when the user closes his/her browser.  You can change this setting by editing <strong>%file</strong>.', array('%life' => ini_get('session.cookie_lifetime'), '%file' => conf_path() .'/settings.php')), 'error');
  }

  $form['persistent_login_welcome'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show welcome message on persistent login.'),
    '#default_value' => variable_get('persistent_login_welcome', TRUE),
    '#description' => t('If checked, the message \'Welcome back, <em>username</em>\' will be displayed each time a new login session is created via a Persistent Login cookie.'),
  );

  $form['persistent_login_maxlife'] = array(
    '#type' => 'textfield',
    '#title' => t('Days to remember the user'),
    '#default_value' => variable_get('persistent_login_maxlife', PERSISTENT_LOGIN_MAXLIFE),
    '#description' => t('The maximum number of days for which a persistent login session is valid; afterwards, the user will need to log in again.  Enter 0 for no expiration.'),
  );

  $form['persistent_login_secure'] = array(
    '#type' => 'radios',
    '#title' => t('Pages which require an explicit login'),
    '#default_value' => variable_get('persistent_login_secure', 1),
    '#options' => array(
      t('Every page except the listed pages.'),
      t('Only the listed pages.'),
    ),
  );

  $form['persistent_login_pages'] = array(
    '#type' => 'textarea',
    '#title' => t('Pages'),
    '#default_value' => variable_get('persistent_login_pages', PERSISTENT_LOGIN_SECURE_PATHS),
    '#cols' => 40,
    '#rows' => 5,
    '#description' => t("Enter one page per line as Drupal paths.  The '*' character is a wildcard.  Example paths are '<em>blog</em>' for the blog page, '<em>blog/*</em>' for every personal blog, and '<em>blog*</em>' for both. '<em>&lt;front&gt;</em>' is the front page."),
  );

  return system_settings_form($form);
}

/**
 * Menu callback to erase a user's Persistent Login records. The menu
 * entry guarantees that the user is logged in.
 *
 * Redirects to user/$uid if allowed or calls drupal_access_denied().
 *
 * @param $uid
 *  The user id for whom to erase PL records.  A user can specify any
 *  uid for the callback, but only the current user's id will work
 *  unless the user has 'administer Persistent Login' permission.
 */
function persistent_login_erase($uid = NULL) {
  global $user;
  if (!isset($uid)) {
    $uid = $user->uid;
  }
  if ($user->uid == $uid || user_access('administer Persistent Login')) {
    _persistent_login_invalidate('erase', 'uid = %d', $uid);
    drupal_goto('user/'. $uid);
  }
  drupal_access_denied();
}

/**
 * After_build callback for login forms.
 */
function persistent_login_form_after_build($form, &$form_state) {
  // FAPI ensures a weight is assigned to all elements before after_build
  // callback is invoked.

  // Get the weight assigned to the submit button.
  $original_submit_weight = $form['submit']['#weight'];

  // Increase the weight of all elements with a weight equal or
  // greater to the weight assigned to the submit button to make
  // room for the "Remember me" checkbox.
  foreach (element_children($form) as $key) {
    if (isset($form[$key]) && $form[$key]) {
      if ($form[$key]['#weight'] >= $original_submit_weight) {
        $form[$key]['#weight'] += 1;
      }
    }
  }

  // Give the "Remember me" checkbox the weight originally assigned
  // to the submit button.
  $form['persistent_login']['#weight'] = $original_submit_weight;

  // Ensure drupal_render() performs the sort by weight step on the form.
  unset($form['#sorted']);

  // Adjust the tabindex of the plain login form.
  if (isset($form['submit']['#attributes']) && isset($form['submit']['#attributes']['tabindex'])) {
    $tabindex = (int)$form['submit']['#attributes']['tabindex'];
    $form['persistent_login']['#attributes']['tabindex'] = $tabindex;
    $form['submit']['#attributes']['tabindex'] = $tabindex + 1;
  }

  return $form;
}
