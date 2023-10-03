/*
   Copyright (c) 2021 Burks Builds
   This software is licensed under the MIT License. See the license file for details.
   Source: https://github.com/burksbuilds/PreferencesCLI
 */
 
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