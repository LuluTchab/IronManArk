#include "ArkConfigMenu.h"


// Class constructor
ArkConfigMenu::ArkConfigMenu()
{
  // Set as "no menu selected"
  _currentMenuIndex = INT_UNINITIALIZED;
  _currentSubMenuIndex = INT_UNINITIALIZED;
  _userInputStep = 0;

  _displayPrompt = true;
  _configFormatIsValid = false;

  _menu[ROOT_MENU_INDEX__WIFI_CONFIG] = "Wifi config";
  _subMenu[ROOT_MENU_INDEX__WIFI_CONFIG][SUB_MENU__BACK] = "Back";
  _subMenu[ROOT_MENU_INDEX__WIFI_CONFIG][getSubMenuIndexFromId(ROOT_MENU_INDEX__WIFI_CONFIG, SUB_MENU__WIFI_CONFIG__VIEW)] = "View current config";
  _subMenu[ROOT_MENU_INDEX__WIFI_CONFIG][getSubMenuIndexFromId(ROOT_MENU_INDEX__WIFI_CONFIG, SUB_MENU__WIFI_CONFIG__LIST_NETWORKS)] = "List available networks";
  _subMenu[ROOT_MENU_INDEX__WIFI_CONFIG][getSubMenuIndexFromId(ROOT_MENU_INDEX__WIFI_CONFIG, SUB_MENU__WIFI_CONFIG__SET)] = "Set SSID/Password";
  
  _menu[ROOT_MENU_INDEX__FONT_CONFIG] = "Font config";
  _subMenu[ROOT_MENU_INDEX__FONT_CONFIG][SUB_MENU__BACK] = "Back";
  _subMenu[ROOT_MENU_INDEX__FONT_CONFIG][getSubMenuIndexFromId(ROOT_MENU_INDEX__FONT_CONFIG, SUB_MENU__FONT_CONFIG__VIEW)] = "View current config";
  _subMenu[ROOT_MENU_INDEX__FONT_CONFIG][getSubMenuIndexFromId(ROOT_MENU_INDEX__FONT_CONFIG, SUB_MENU__FONT_CONFIG__TOGGLE_BLINK)] = "Toggle blinking colon";

}

// ------------------------------------------------------------
// Start the configuration
// NOTE: we cannot do this inside class constructor because it won't be
// able to correcly load configuration from Preferences
void ArkConfigMenu::begin()
{
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }

  loadConfig();
  
  // At beginning, we expect for INT to navigate into menu
  _nextExpectedInputTypePrompt = EXPECTED_INPUT_TYPE_PROMPT__INT;
  _nextInputPrompt = INPUT_PROMPT__CHOICE;
  // We start by displaying the menu
  printCurrentMenu();

  Serial.setTimeout(10);
}

// ------------------------------------------------------------
// To tell if configuration format is valid
bool ArkConfigMenu::isConfigFormatValid() { return _configFormatIsValid; }


// ------------------------------------------------------------
// Handle menu inputs
short ArkConfigMenu::handleInput()
{
  // Id of action to return to caller, in case of something has to be handled on caller side
  short actionToReturn = INT_UNINITIALIZED;

  if(_displayPrompt)
  {
    _displayPrompt = false;
    displayNextInputPrompt();
    
  }
  else
  {
    if (Serial.available() > 0)
    {
      String userInput = Serial.readString();
      userInput.trim();

      // Help is requested (we print current menu)
      if(userInput == "?")
      {
        printCurrentMenu();
      }
      else // Help wasn't requested
      {

        // if we are navigating into menus
        if(_currentMenuIndex==INT_UNINITIALIZED || _currentSubMenuIndex==INT_UNINITIALIZED)
        {
          int requestedMenuIndex = convertCharToInt(userInput[0]);

          // if root menu is displayed but nothing is selected
          if(_currentMenuIndex == INT_UNINITIALIZED)
          {
            // If we are in range, we select menu
            if(requestedMenuIndex < ROOT_MENU_SIZE) _currentMenuIndex = requestedMenuIndex;

            printCurrentMenu();
          }
          // if submenu is displayed but nothing selected
          else if(_currentSubMenuIndex==INT_UNINITIALIZED)
          {
            // If we are in range, we select menu
            if(requestedMenuIndex < SUB_MENU_SIZE)
            {
              // If we asked for 'Back'
              if(requestedMenuIndex == SUB_MENU__BACK)
              {
                // We go back to root menu
                _currentMenuIndex = INT_UNINITIALIZED;
                printCurrentMenu();
              }
              else // Another menu entry was selected
              {
                _currentSubMenuIndex = requestedMenuIndex;
                actionToReturn = handleSubMenu(userInput);
              }
              
            }
            else // selected sub-menu is out of range
            {
              printCurrentMenu();
            }
          }
          
        }// END if we are navigating into menus        
        else // We've selected an entry in sub-menu
        {
          actionToReturn = handleSubMenu(userInput);
        }
      }// END if help wasn't requested
      
      _displayPrompt = true;

    }// END if serial is avaiable
  }// END if we are waiting for user input

  return actionToReturn;
}


// ------------------------------------------------------------
// Set next input prompt text and expected type
void ArkConfigMenu::setNextInputPrompt(String txt, char type)
{
  _nextInputPrompt = txt;
  _nextExpectedInputTypePrompt = type;
  // To display prompt at next occurrence
  _displayPrompt = true;
}


// ------------------------------------------------------------
// Displays next user input prompt, with char to identify type
void ArkConfigMenu::displayNextInputPrompt()
{
  Serial.print(_nextInputPrompt);
  Serial.println(_nextExpectedInputTypePrompt);
}


// ------------------------------------------------------------
// Returns Configuration information
char* ArkConfigMenu::getWifiSSID() { return _config.wifi.ssid; }
char* ArkConfigMenu::getWifiPassword() { return _config.wifi.password; }
bool ArkConfigMenu::doesColonHaveToBlink() { return _config.font.colonBlink; }

// ------------------------------------------------------------
// Converts char to Int
short ArkConfigMenu::convertCharToInt(char c)
{
  if(c=='0') return 0;
  if(c=='1') return 1;
  if(c=='2') return 2;
  if(c=='3') return 3;
  if(c=='4') return 4;
  if(c=='5') return 5;
  if(c=='6') return 6;
  if(c=='7') return 7;
  if(c=='8') return 8;  
  if(c=='9') return 9;

  return -1;
}

// ------------------------------------------------------------
// Converts String into char*
char* ArkConfigMenu::convertStringToCharArray(String text)
{
  char result[text.length()+1];
  text.toCharArray(result, text.length());
  return result;
}


// ------------------------------------------------------------
// Display menu for user
void ArkConfigMenu::printCurrentMenu()
{
  // If we have to display Root menu
  if(_currentMenuIndex == INT_UNINITIALIZED)
  {
    Serial.println("");
    Serial.println("= Select option =");
    for(short i=0; i<ROOT_MENU_SIZE; i++)
    {
      Serial.print("["); Serial.print(i); Serial.print("] "); Serial.println(_menu[i]);
    }
    Serial.println("");
  }
  // We have to display one of the sub menus
  else if(_currentSubMenuIndex == INT_UNINITIALIZED)
  {
    Serial.println("");
    // First we display root menu name
    Serial.print("= "); Serial.print(_menu[_currentMenuIndex]); Serial.println(" =");
    for(short i=0; i<SUB_MENU_SIZE; i++)
    {
      Serial.print("["); Serial.print(i); Serial.print("] "); Serial.println(_subMenu[_currentMenuIndex][i]);
    }
    Serial.println("");
  }

  // Initializing
  setNextInputPrompt(INPUT_PROMPT__CHOICE, EXPECTED_INPUT_TYPE_PROMPT__INT);

}


// ------------------------------------------------------------
// Handle sub-menu selection
short ArkConfigMenu::handleSubMenu(String lastUserInput)
{
  // Id of action to return to caller, in case of something has to be handled on caller side
  short actionToReturn = INT_UNINITIALIZED;

  // Depending sub-menu that was selected
  switch(getSubMenuIdFromIndex(_currentMenuIndex, _currentSubMenuIndex))
  {
    // --------------------
    // Wifi - View Config
    case SUB_MENU__WIFI_CONFIG__VIEW:
    {
      if(_configFormatIsValid)
      {
        Serial.println("== Wifi Config ==");
        Serial.print(" SSID: "); Serial.println(_config.wifi.ssid);
      }
      else
      {
        displayInvalidConfigMessage();
      }
      // To display sub-menu again
      _currentSubMenuIndex = INT_UNINITIALIZED;
      break;
    }

    // --------------------
    // Wifi - list networks
    case SUB_MENU__WIFI_CONFIG__LIST_NETWORKS:
    {
      // We gave back to caller the action that was selected
      actionToReturn = SUB_MENU__WIFI_CONFIG__LIST_NETWORKS;
      // To display sub-menu again
      _currentSubMenuIndex = INT_UNINITIALIZED;
      break;
    }

    // --------------------
    // Wifi - Set Config
    case SUB_MENU__WIFI_CONFIG__SET:
    {
      switch(_userInputStep)
      {
        // We have to ask for Wifi SSID
        case 0:
        {
          setNextInputPrompt("Enter Wifi SSID", EXPECTED_INPUT_TYPE_PROMPT__STRING);
          break;
        }
        // We have to ask for Wifi Password
        case 1:
        {
          // Saving Wifi SSID that was just given
          
          lastUserInput.toCharArray(_config.wifi.ssid, lastUserInput.length()+1);
          setNextInputPrompt("Enter Wifi Password", EXPECTED_INPUT_TYPE_PROMPT__PASSWORD);
          break;
        }
        // Both SSID and password were given
        case 2:
        {
          // Saving Wifi Password that was just given
          lastUserInput.toCharArray(_config.wifi.password, lastUserInput.length()+1);
          // Saving configuration into EEPROM
          saveConfig();
          Serial.println("Ark reactor will restart in 2 seconds...");
          delay(2000);
          ESP.restart();
          // To display sub-menu again
          _currentSubMenuIndex = INT_UNINITIALIZED;
          break;
        }
      }
      break;
    }
  
    // --------------------
    // Font - View Config
    case SUB_MENU__FONT_CONFIG__VIEW:
    {
      Serial.println("== Font Config ==");
      Serial.print(" Colon is blinking: "); Serial.println((_config.font.colonBlink)?"Yes":"No");
      // To display sub-menu again
      _currentSubMenuIndex = INT_UNINITIALIZED;
      break;
    }

    // --------------------
    // Font - Toggle blink
    case SUB_MENU__FONT_CONFIG__TOGGLE_BLINK:
    {
      
      switch(_userInputStep)
      {
        // We have to ask for Colon blinking or not
        case 0:
        {
          setNextInputPrompt("Colon blinking enabled [y/n]", EXPECTED_INPUT_TYPE_PROMPT__BOOL);
          break;
        }
        // Info about colon blinking has been given
        case 1:
        {
          lastUserInput.toLowerCase();
          _config.font.colonBlink = (lastUserInput == "y");
          saveConfig();
          // To display sub-menu again
          _currentSubMenuIndex = INT_UNINITIALIZED;
          break;
        }
      }
      break;
    }

  }

  // to go to next input step if needed
  if(_currentSubMenuIndex != INT_UNINITIALIZED)
  {
    _userInputStep++;
  }
  else
  {
    _userInputStep = 0;
  }

  // Display menu if needed
  if(_currentSubMenuIndex == INT_UNINITIALIZED) printCurrentMenu();

  return actionToReturn;
  
}


// ------------------------------------------------------------
void ArkConfigMenu::displayInvalidConfigMessage()
{
  Serial.println("!! Configuration is invalid, please update it !!");
}


// ------------------------------------------------------------
// Returns array index in which menu with ID menuIndex is located
// rootMenuIndex can be ROOT_MENU_INDEX__WIFI_CONFIG or ROOT_MENU_INDEX__FONT_CONFIG
// subMenuId can be value of SUB_MENU__*, except SUB_MENU__BACK
short ArkConfigMenu::getSubMenuIndexFromId(short rootMenuIndex, short subMenuId)
{
  return subMenuId - (rootMenuIndex*10);
}

// ------------------------------------------------------------
// Returns menu ID (value of SUB_MENU__*, except SUB_MENU__BACK) for given menu index
// rootMenuIndex can be ROOT_MENU_INDEX__WIFI_CONFIG or ROOT_MENU_INDEX__FONT_CONFIG
// subMenuIndex, from 0 to  <ROOT_MENU_SIZE
short ArkConfigMenu::getSubMenuIdFromIndex(short rootMenuIndex, short subMenuIndex)
{
  return subMenuIndex + (rootMenuIndex*10);
}


// ------------------------------------------------------------
// Load configuration from Preferences
void ArkConfigMenu::loadConfig()
{
  Preferences prefs;
  String tmp;
  prefs.begin(CONFIG_NAMESPACE, true);
  _config.version = prefs.getShort(CONFIG_OPTION__CONFIG_VERSION, INT_UNINITIALIZED);
  // Wifi
  tmp = prefs.getString(CONFIG_OPTION__WIFI__SSID, STRING_UNINITIALIZED);
  tmp.toCharArray(_config.wifi.ssid, tmp.length()+1);
  tmp = prefs.getString(CONFIG_OPTION__WIFI__PASSWORD, STRING_UNINITIALIZED);
  tmp.toCharArray(_config.wifi.password, tmp.length()+1);
  // Font
  _config.font.colonBlink = prefs.getBool(CONFIG_OPTION__FONT__BLINKING_COLON, false);
  prefs.end();

  _configFormatIsValid = true;
  // If equals to 0, it means that we don't have information about that
  if(_config.version == INT_UNINITIALIZED)
  {
    _configFormatIsValid = false;
  }

  // If stored configuration version is different than the current one
  if(_config.version != ARK_CONFIG_VERSION)
  {
    // TODO: handle in the future
    _configFormatIsValid = false;
  }
}


// ------------------------------------------------------------
// Saves configuration into Preferences
void ArkConfigMenu::saveConfig()
{
  Preferences prefs;
  prefs.begin(CONFIG_NAMESPACE, false);

  prefs.putShort(CONFIG_OPTION__CONFIG_VERSION, ARK_CONFIG_VERSION);
  // Wifi
  prefs.putString(CONFIG_OPTION__WIFI__SSID, _config.wifi.ssid);
  prefs.putString(CONFIG_OPTION__WIFI__PASSWORD, _config.wifi.password);
  // Font
  prefs.putBool(CONFIG_OPTION__FONT__BLINKING_COLON, _config.font.colonBlink);

  prefs.end();
  // Because we saved it, config format is now valid again
  _configFormatIsValid = true;
}


