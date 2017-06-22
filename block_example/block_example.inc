<?php

/**
 * BlockExample extends Block
 *
 * The Block class provides methods to provide blocks without utilizing the
 * Backdrop hook system. The methods provided here allow you to define the
 * labels, forms, validation, and display of blocks.
 *
 * Blocks defined by extending the Block class cannot be "_altered" in the
 * traditional manner, however this class can be extended, and the methods
 * overridden, to introduce new functionality.
 */
class BlockExample extends Block {

  /**
   * Sets title text on draggable block panel in Layout builder.
   */
  function getAdminTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Block Example block');
  }

  /**
   * Sets block subject on block view.
   */
  function getTitle() {
    return isset($this->settings['title']) ? check_plain($this->settings['title']) : '';
  }

  /**
   * Returns the rendered content of this block.
   *
   * @return string
   */
  function getContent() {
    if (!empty($this->settings['content'])) {
      $intro_text = t("The following is the content of the example subclass block:\n");
      $entered_content = check_markup(token_replace($this->settings['content']), $this->settings['format'], '', TRUE);
      return ($intro_text . $entered_content);
    }
  }

  /**
   * Builds the block's configuration form.
   *
   * The values of the form elements defined here will be merged with defaults
   * from the parent class (Block class in this case) and made available in
   * the formValidate() and formSubmit() functions.
   *
   * Block class adds default form elements to allow users to change the 
   * block's title. If you do not wish for users to be able to define custom 
   * block titles, you can set access to the title elements to false, and then
   * define a custom title element, as in the following example:
   *
   *   $form['title_display']['title_display']['#access'] = FALSE;
   *   $form['title_display']['title'] = array(
   *     '#type' => 'textfield',
   *     '#title' => t('Display title'),
   *     '#default_value' => $this->settings['title'],
   *   );
   *
   * You would then need to explictly save a value for 'title' in formSubmit().
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $form['content'] = array(
      '#type' => 'text_format',
      '#title' => t('Block content'),
      '#default_value' => !empty($this->settings['content']) ? $this->settings['content'] : '',
      '#format' => !empty($this->settings['format']) ? $this->settings['format'] : 'filtered_html',
      '#editor_uploads' => TRUE,
      '#rows' => 5,
    );

  }

  /**
   * Validates the form settings.
   */
  function formValidate($form, &$form_state) {
    parent::formValidate($form, $form_state);

    // We'll make it a requirement to include some content.
    if (empty($form_state['values']['content'])) {
      form_error($form['content'], t('Please enter some content.'));
    }
  }

  /**
   * Submit handler to save the form settings.
   *
   * All keys included in $this->settings are automatically saved to the
   * configuration file for the Layout where this block instance has been
   * placed. It is not necessary to return a value from formSubmit().
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    $this->settings['content'] = $form_state['values']['content']['value'];
    $this->settings['format'] = $form_state['values']['content']['format'];
  }
}
