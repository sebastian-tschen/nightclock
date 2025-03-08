
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
      - allTimeHighScore:
          label: all time highscore
          attribs: "min='0' max='99' step='1'"
      - dailyHighScoreDate:
          label: date of last dailyHighscore
      - dailyHighScore:
          label: last dailyHighscore
          attribs: "min='0' max='99' step='1'"
      - hueOffset:
          label: Clock Hue Offset
          attribs: "min='0' max='255'"
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
    