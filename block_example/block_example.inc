<?php

/**
 *  BlockExampleCustomClass extends Block
 *  
 *  This class allows us to create a block, set up the administrative interface 
 *  in Layout builder and set up configuration.
 */
class BlockExampleCustomClass extends Block {
  /**
   *  Sets title text on draggable block panel in Layout builder.
   *  
   *  Defaults to the 'title' attribute set in hook_block_info
   */
  function getAdminTitle() {
    return t('Example: uses custom class');
  }

  /**
   *  Sets description text on draggable block in Layout builder.
   *  
   *  Defaults to the 'description' attribute set in hook_block_info
   */
  function getAdminPreview() {
    return t('This block uses a custom class.');
  }

  /**
   *  Sets block subject on block view.
   */
  function getTitle() {
    return t('Title of custom class block (example_custom_class)');
  }

  /**
   *  Sets block content on block view.
   */
  function getContent() {
    $content = !empty($this->settings['custom_class_string']) ? $this->settings['custom_class_string'] : 'No content saved';
    return $content;
  }

  /**
   *  Builds the block's configuration form.
   */
  function form(&$form, &$form_state) {
    $default_value = isset($this->settings['custom_class_string']) ? $this->settings['custom_class_string'] : array();
    $form['custom_class_string'] = array(
      '#type' => 'textfield',
      '#title' => t('Block contents'),
      '#size' => 60,
      '#description' => t('This text will appear in the example block.'),
      '#default_value' => $default_value,
    );
  }
  
  /**
   * Submit handler to save the form settings.
   *
   * Settings are saved to the active layout configuration file.
   */
  function formSubmit($form, &$form_state) {
    $this->settings['custom_class_string'] = $form_state['values']['custom_class_string'];
  }
}
/**
 * @} End of "defgroup block_example".
 */
