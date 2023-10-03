/*
   Copyright (c) 2021 Burks Builds
   This software is licensed under the MIT License. See the license file for details.
   Source: https://github.com/burksbuilds/PreferencesCLI
 */
 
#ifndef PREFERENCES_CLI_H
#define PREFERENCES_CLI_H

#include "Preferences.h"
#include "SimpleCLI.h"

#ifndef PREFERENCES_CLI_RESPONSE_BUFFER_SIZE
#define PREFERENCES_CLI_RESPONSE_BUFFER_SIZE 256
#endif

using CommandCallbackWrapper = std::function<void(cmd*)>;

class PreferencesCLI
{
	private:
		Preferences& _preferences;
		
		char streamResponseBuffer[PREFERENCES_CLI_RESPONSE_BUFFER_SIZE];
		
		Command getPreferenceCommand;
		Command setPreferenceCommand;
		Command clearPreferenceCommand;
		
		void getPreferenceCallback(cmd* commandPointer, Stream &outputStream);
		void setPreferenceCallback(cmd* commandPointer, Stream &outputStream);
		void clearPreferenceCallback(cmd* commandPointer, Stream &outputStream);
		
		static const char* getPreferenceTypeName(PreferenceType pType);
		static PreferenceType getPreferenceType(const char* typeName);
		static uint8_t parseHexChar(char hex);
		static char getHexChar(uint8_t val);

	
	public:
		PreferencesCLI(Preferences &preferences);
		void registerCommands(SimpleCLI &cli);
    bool handleCommand(Command &command, Stream &outputStream);
		
};

#endif