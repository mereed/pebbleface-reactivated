module.exports=[
    {
        "type": "heading",
        "defaultValue": "Configuration"
    },
    {
        "type": "text",
        "defaultValue": "<h6>A white button = OFF <br> An orange button = ON</h6>",
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "<h5>Weather</h5>"
            },
            {
                "type": "input",
                "messageKey": "location",
                "label": "Location",
                "description": "Enter a location for \"Use GPS\" NO. ie. city name or postal/zip code.",
                "defaultValue": ""
            },
            {
                "type": "toggle",
                "messageKey": "use_gps",
                "label": "Use GPS",
                "description": "If you want to conserve battery, set GPS=NO AND enter your location eg. London.",
                "defaultValue": true
            },
            {
                "type": "radiogroup",
                "messageKey": "units",
                "label": "Temp. Units",
                "defaultValue": "fahrenheit",
                "options": [
                    {
                        "label": "Celsius",
                        "value": "celsius"
                    },
                    {
                        "label": "Fahrenheit",
                        "value": "fahrenheit"
                    }
                ]
            },
        ]
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "<h5>Display</h5>"
            },
            {
                "type": "select",
                "messageKey": "row1right",
                "label": " Middle Right =",
  				"capabilities": ["HEALTH", "NOT_ROUND"],
				"defaultValue": "0.",
                "options": [
 
                    {
                        "label": "Health Arcs",
                        "value": "0."
                    },
			        {
                        "label": "Step Arcs",
                        "value": "1."
                    },
			                   {
                        "label": "Step Count",
                        "value": "2."
                    },
			                   {
                        "label": "Heart Rate (P2HR)",
                        "value": "3."
                    }
		]
            },
	            {
                "type": "select",
                "messageKey": "row2left",
                "defaultValue": "0.",
                "label": "Lower Right Cnr =",
  				"capabilities": ["HEALTH", "NOT_ROUND"],
                "options": [
 
                     {
                        "label": "Health Arcs",
                        "value": "0."
                    },
                    {
                       "label": "Step Arcs",
                        "value": "1."
                    },
					{
                        "label": "Step Count",
                        "value": "2."
                    },
					{
                        "label": "Heart Rate (P2HR)",
                        "value": "3."
                    }
                   
                ]
            },			
					 {
                "type": "radiogroup",
                "messageKey": "fill",
                "label": "Time style",
                "defaultValue": "0.",
                "options": [
                    {
                        "label": "Solid fill",
                        "value": "1."
                    },
                    {
                        "label": "Outline",
                        "value": "0."
                    }
                ]
            },
	  //         {
      //          "type": "toggle",
     //           "messageKey": "weather_status",
      //          "label": "12h mode",
      //          "defaultValue": false
      //      },
	
        ]
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "<h5>Vibration</h5>"
            },
            {
                "type": "toggle",
                "messageKey": "bluetoothvibe_status",
                "label": "Bluetooth vibration",
                "defaultValue": false
            },
            {
                "type": "toggle",
                "messageKey": "hourlyvibe_status",
                "label": "Hourly vibration",
                "defaultValue": false
            }
        ]
    },
    {
        "type": "submit",
        "defaultValue": "Save Settings"
    },
    {
        "type": "text",
        "defaultValue": "<h6><center>If you find this watchface useful, <br> please consider making a <a href='https://www.paypal.me/markchopsreed'>small donation</a>. <br> Thank you. </center></h6>"
    }
];