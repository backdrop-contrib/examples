/**
 * Provides a farbtastic colorpicker for the fancier widget.
 */
(function ($) {
  Backdrop.behaviors.field_example_colorpicker = {
    attach: function(context) {
      $('.node-form').on('focus', '.edit-field-example-colorpicker', function(event){
        var edit_field = this;
        var picker = $(this).closest('div').parent().find(".field-example-colorpicker");
        
        // Hide all color pickers except this one.
        $(".field-example-colorpicker").hide();
        $(picker).show();
        $.farbtastic(picker, function(color) {
          edit_field.value = color;
        }).setColor(edit_field.value);
      });
    }
  }
})(jQuery);
