# PreferencesCLI

Arduino library to allow access to the [Preferences library](https://github.com/vshymanskyy/Preferences) via the [SimpleCLI library](https://github.com/SpacehuhnTech/SimpleCLI)

## User Interaction

This library lets you use any Stream (Serial, BTSerial, etc) as a command line interface using the [SimpleCLI Library](https://github.com/SpacehuhnTech/SimpleCLI). 
It exposes commands to get, set, and clear entries from the on-board Preferences (non-volatile storage). 
The user types commands in and sends them through their serial interface. 
The library parses the command, interacts with the onboard Preferences, then sends a response message back through the same serial interface.

### Set Preference Value

Used to write the value of entries using the built-in 'Preferences' library. 
Note that you do not need to specify each parameter name (such as namespace or key) so long as the order of the parameters given matches the command description.

`setP/ref/erence,setp -namespace,ns <value> -k/ey <value> -t/ype <value> -v/al/ue <value>`
- namespace (-ns): the namespace of the preference setting
- key (-k): the string identifier of the preference
- type (-t): the type of the data to write (Bool, U/Char, U/Int8, U/Int16, U/Int32, U/Int64, String, Float, Double, Bytes)
- value (-v): the value to store (Bytes parsed as HEX)

Example to set a float: `> setp Pot1 CutoffVoltage Float 1.45`

Example to set an int: `> setp Pot1 PinNumber Int16 4` 
- note: negative numbers must be enclosed in "quotes" to avoid being parsed as a flag argument.

Example to set a string: `> setp Wifi SSID String "Esp Access Point"`
- note: text with spaces must be enclosed in "quotes" to avoid being parsed as multiple arguments.

Example to set a byte array: `> setp EspNow PeerMacAdr Bytes 08D45E68A232`

### Get Preference Value

Used to read the value of entries using the built-in 'Preferences' library.
Note that you do not need to specify each parameter name (such as namespace or key) so long as the order of the parameters given matches the command description.
If the type is omitted then the library returns information about the type of the key instead of the value of the key.
If the key does not exist then the library will notify you.

`getP/ref/erence,getp -namespace,ns <value> -k/ey <value> [-t/ype <value>]`
- namespace (-ns): the namespace of the preference setting
- key (-k): the string identifier of the preference
- type (-t): the type of the data to read (Bool, U/Char, U/Int8, U/Int16, U/Int32, U/Int64, String, Float, Double, Bytes) (omit to return the type itself)

Example to get an unknown key type (Returns `Float/Double/Bytes (4 Bytes)`: `> getp Pot1 CutoffVoltage`
- note: Floats and Doubles are stored in the same way as byte strings under the hood, using 4 and 8 bytes of space respectively.

Example to get a float (Returns `1.450000`): `> getp Pot1 CutoffVoltage Float`

Example to get a string (Returns `Esp Access Point`): `> getp Wifi SSID String`

Example to get a byte array (Returns `08D45E68A232`): `> getp EspNow PeerMacAdr Bytes`

Example attempting to get an uninitialized parameter (Returns `ERROR: unable to locate key 'Password' in namespace 'Wifi'`): `> getp Wifi Password String`

### Clear Preferences

Used to clear individual preferences, entire namespaces, or the entire nvs storage area.
Note that you do not need to specify each parameter name (such as namespace or key) so long as the order of the parameters given matches the command description.
If just the key is omitted then the entire namespace is cleared.
If the key and namespace are both omitted then the entire NVS storage area is cleared (ESP32 only, may have unintended consequences).

`clearP/ref/erence,clearp [-namespace,ns <value>] [-k/ey <value>]`
- namespace (-ns): the namespace of the preference setting (omit to clear entire NVS area)
- key (-k): the string identifier of the preference (omit to clear entire namespace)

Example to remove a string: `> clearp Wifi Password`

Example to remove a namespace: `> clearp Pot1`

## Implementation

Implementation of the library within your own sketch is done in three parts:
1. Initialize an instance of the PreferencesCLI class and associate it with a Preferences object, probably as a global: `PreferencesCLI prefCli(preferencesInstance);`
 - Note that you probably also have a Serial global and SimpleCLI global that will be used later.
2. Bind the preference related commands to the SimpleCLI object once, probably in the setup function: `prefCli.registerCommands(simpleCliInstance);`
 - Note that the SimpleCLI object can have other commands registered for other purposes, and it will not interfere with this library
3. Continuously check if the CLI has any available (active) commands, and if so pass them into the library for evaluation, probably in the loop function: `prefCli.handleCommand(commandInstance);`

Check out [this example sketch](examples/usbSerialCLI) of a basic implementation over USB serial:
```
#include "Preferences.h"
#include "SimpleCLI.h"
#include "PreferencesCLI.h"

//Initialize global objects
Preferences preferences;
SimpleCLI cli;
PreferencesCLI prefCli(preferences);

void setup() {
  Serial.begin(115200);
  Serial.print("> ");

  //register all of the preference related commands with the CLI. Other commands could be registered for other reasons too.
  prefCli.registerCommands(cli);
}

void loop() {

  //Check if the user has sent text in via serial, then have the CLI parse it as a command
  if(Serial.available())
  {
    String line = Serial.readStringUntil('\n');
    Serial.println(line);
    cli.parse(line);
  }

  //If the user input was successfulyl parsed by the CLI as a known command then it will be available in the CLI's queue
  if(cli.available())
  {
    bool handled = false;
    //'pop' the first command from the CLI parsed command queue
    Command command = cli.getCommand();
    //check to see if the command is related to the preferences library and handle it if it is.
    //The command might not be for the preferences library, so you may need to go 'ask' other parts of your code if they can handle this command.
    handled &= prefCli.handleCommand(command, Serial);
    Serial.print("\n> ");
  }

  //if the input passed into the CLI parser is not a valid command then it can be popped off the error queue
  if(cli.errored())
  {
    CommandError e = cli.getError();
    handleCliError(e, Serial);
  }
}

//this function handles displaying an error message over serial if the text sent does not parse as a known, valid command
void handleCliError(CommandError &e, Stream &outputStream)
{
  CommandError cmdError(e);
  outputStream.print("ERROR: ");
  outputStream.println(cmdError.toString());

  //This is the case where the CLI was smart enough to know what command was requested, but the parameters sent with the command don't make sense for that command
  if (cmdError.hasCommand()) {
      outputStream.println(cmdError.getCommand().toString());
  }
  outputStream.print("\n> ");
}
```

## Compatability

- The driving factor for compatability is the Preferences library itself.
  - ESP32 boards use the built in [ESP32 preferences API](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/preferences.html).
  - Other boards (ESP8266, RP2040, Realtek, Particle Gen3) use the [Arduino Preferences Library](https://github.com/vshymanskyy/Preferences) (see their documentation for more details on the hardware restrictions).
- In order to clear all preferences from all namespaces, the NVS storage is wiped in its entirety (which could have unintended consequences). This is only possible on boards with NVS storage, so it is locked behind an ESP32 compiler flag for now.

## Shortcomings

Because there is no built in way to parse a string as a UInt32, UInt64, or Int64 those data types are currently not supported with the setPreferences function.

The underlying SimpleCLI library prefers using callback functions to evaluate commands.
It is tricky to use an instance-member function as a callback function. 
The <functional> library probably has a work around with std::function and std::bind but I do not understand how to use them.
The underlying library also does not have a 'peek' function to check the next command in the queue, 
	although it does have a 'pop' function that applies when you query the command queue for a specific command.
The end result is that I could not figure out a good way to automatically evaluate commands from the SimpleCLI queue 
	(while maintaining the ability to use other commands in the CLI)
	without having the user code pop the top command from the queue and evaluate it.

## Contributing

Make a pull request if you have a good idea to fix the above issues!

The [loopback](examples/loopback) example is the method used to evaluate if the library 'works'.