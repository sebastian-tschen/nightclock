
// Default application config dictionary
// Modify the file with the params for you application
// Then you can use then then by val = config[name];
const char* VARIABLES_DEF_YAML PROGMEM = R"~(
    Wifi settings:
      - st_ssid:
          label: Name for WLAN
      - st_pass:
          label: Password for WLAN
      - host_name:
          label: Host name to use for MDNS and AP
            {mac} will be replaced with device's mac id
          default: configAssist_{mac}
    
    Application settings:
      - app_name:
          label: Name your application
          default: ConfigAssistDemo
      - led_buildin:
          label: Enter the pin that the build in led is connected.
            Leave blank for auto.
          attribs: "min='2' max='23' step='1'"
      - debug:
          label: Debug application
          checked: true      
    
    ConfigAssist settings:
      - display_style:
          label: Choose how the config sections are displayed.
            Must reboot to apply
          options:
            - AllOpen: 0
            - AllClosed: 1
            - Accordion : 2
            - AccordionToggleClosed : 3
          default: AccordionToggleClosed  -
    )~";
    