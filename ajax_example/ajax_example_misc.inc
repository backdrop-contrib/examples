<?php
/**
 * @file
 * AJAX Miscellaneous Topics.
 */

/**
 * Demonstrates a clickable AJAX-enabled link using the 'use-ajax' class.
 *
 * Because of the 'use-ajax' class applied here, the link submission is done
 * without a page refresh.
 *
 * When using the AJAX framework outside the context of a form or a renderable
 * array of type 'link', you have to include ajax.js explicitly.
 *
 * @return string
 *   The HTML markup for the rendered link.
 *
 * @ingroup ajax_example
 */
function ajax_example_render_link() {
  // backdrop_add_library is invoked automatically when a form element has the
  // '#ajax' property, but since we are not rendering a form here, we have to
  // do it ourselves.
  backdrop_add_library('system', 'backdrop.ajax');
  $explanation = t("
The link below has the <i>use-ajax</i> class applied to it, so if
JavaScript is enabled, ajax.js will try to submit it via an AJAX call instead
of a normal page load. The URL also contains the '/nojs/' magic string, which
is stripped if JavaScript is enabled, allowing the server code to tell by the
URL whether JavaScript was enabled or not, letting it do different things based on that.");
  $output = "<div>" . $explanation . "</div>";
  // The use-ajax class is special; that the link will call without causing a
  // page reload. Note the /nojs portion of the path; when JavaScript is
  // enabled, this part will be stripped from the path before it is called.
  $link = l(t('Click here'), 'ajax_link_callback/nojs/', array('attributes' => array('class' => array('use-ajax'))));
  $output .= "<div id='myDiv'></div><div>$link</div>";
  return $output;
}

/**
 * AJAX-enabled link in a renderable array.
 *
 * Demonstrates a clickable AJAX-enabled link using a renderable array with the
 * #ajax property.
 *
 * A link that is constructed as a renderable array can have the #ajax property,
 * which ensures that the link submission is done without a page refresh. The
 * href of the link is used as the ajax callback, but it degrades gracefully
 * without JavaScript because if the 'nojs' portion of the href is not stripped
 * out by js, the callback will return content as required for a full page
 * reload.
 *
 * The necessary JavaScript file, ajax.js, will be included on the page
 * automatically.
 *
 * @return array
 *   Form API array.
 */
function ajax_example_render_link_ra() {
  $explanation = "
The link below has been rendered as an element with the #ajax property, so if
JavaScript is enabled, ajax.js will try to submit it via an AJAX call instead
of a normal page load. The URL also contains the '/nojs/' magic string, which
is stripped if JavaScript is enabled, allowing the server code to tell by the
URL whether JavaScript was enabled or not, letting it do different things based on that.";
  $build['my_div'] = array(
    '#markup' => $explanation . '<div id="myDiv"></div>',
  );
  $build['ajax_link'] = array(
    '#type' => 'link',
    '#title' => t('Click here'),
    // Note the /nojs portion of the href; when JavaScript is enabled, this part
    // will be stripped from the path before it is called.
    '#href' => 'ajax_link_callback/nojs/',
    '#id' => 'ajax_link',
    '#ajax' => array(
      'wrapper' => 'myDiv',
      'method' => 'html',
    ),
  );
  return $build;
}

/**
 * Callback for link example.
 *
 * Takes different logic paths based on whether JavaScript was enabled.
 * If $type == 'ajax', it tells this function that ajax.js has rewritten
 * the URL and thus we are doing an AJAX and can return an array of commands.
 *
 * @param string $type
 *   Either 'ajax' or 'nojs. Type is simply the normal URL argument to this URL.
 *
 * @return string|array
 *   If $type == 'ajax', returns an array of AJAX Commands.
 *   Otherwise, just returns the content, which will end up being a page.
 *
 * @ingroup ajax_example
 */
function ajax_link_response($type = 'ajax') {
  if ($type == 'ajax') {
    $output = t("This is some content delivered via AJAX");
    $commands = array();
    // See ajax_example_advanced.inc for more details on the available commands
    // and how to use them.
    $commands[] = ajax_command_append('#myDiv', $output);
    $page = array('#type' => 'ajax', '#commands' => $commands);
    ajax_deliver($page);
  }
  else {
    $output = t("This is some content delivered via a page load.");
    return $output;
  }
}
