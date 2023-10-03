/*
   Copyright (c) 2021 Burks Builds
   This software is licensed under the MIT License. See the license file for details.
   Source: https://github.com/burksbuilds/PreferencesCLI
 */
 
#include "Preferences.h"
#include "SimpleCLI.h"
#include "PreferencesCLI.h"

Preferences preferences;
SimpleCLI cli;
PreferencesCLI prefCli(preferences);

void setup() {
  Serial.begin(115200);
  Serial.print("> ");
  prefCli.registerCommands(cli);
}

void loop() {
  if(Serial.available())
  {
    String line = Serial.readStringUntil('\n');
    Serial.println(line);
    cli.parse(line);
  }

  if(cli.available())
  {
    bool handled = false;
    Command command = cli.getCommand();
    handled &= prefCli.handleCommand(command, Serial);
    Serial.print("\n> ");
  }

  if(cli.errored())
  {
    CommandError e = cli.getError();
    handleCliError(e, Serial);
  }
}
void handleCliError(CommandError &e, Stream &outputStream)
{
  CommandError cmdError(e);
  outputStream.print("ERROR: ");
  outputStream.println(cmdError.toString());

  if (cmdError.hasCommand()) {
      outputStream.println(cmdError.getCommand().toString());
  }
  outputStream.print("\n> ");
}