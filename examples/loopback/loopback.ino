/*
   Copyright (c) 2021 Burks Builds
   This software is licensed under the MIT License. See the license file for details.
   Source: https://github.com/burksbuilds/PreferencesCLI
 */
 
#include "Preferences.h"
#include "SimpleCLI.h"
#include "PreferencesCLI.h"
#include <LoopbackStream.h>

LoopbackStream testStream(PREFERENCES_CLI_RESPONSE_BUFFER_SIZE);
Preferences preferences;
SimpleCLI cli;
PreferencesCLI prefCli(preferences);



void setup() {
  Serial.begin(115200);
  prefCli.registerCommands(cli);
  preferences.begin("ns");
  preferences.clear();
  preferences.end();
}

void loop() {
  if(Serial.available())
  {
    Serial.readString();
    Serial.println("Running tests...");
    if(runTests())
    {
      Serial.println("All tests passed!");
    } else {
      Serial.println("At least one test did not pass.");
    }
  }
}

bool runTests()
{
  bool allPass = true;

  //test basic retreival and clearing
  allPass &= testPrompt("getp ns k","ERROR: unable to locate key 'k' in namespace 'ns'");
  allPass &= testPrompt("setp ns k Bool TRUE","'TRUE' stored as a Bool (1 Bytes) in ns/k");
  allPass &= testPrompt("getp ns k","UInt8/UChar/Bool");
  allPass &= testPrompt("clearp ns k","'k' has been cleared from namespace 'ns'");
  allPass &= testPrompt("getp ns k","ERROR: unable to locate key 'k' in namespace 'ns'");

  //test storing every data type
  allPass &= testPrompt("setp ns kChar Char a","'a' stored as a Char (1 Bytes) in ns/kChar");
  allPass &= testPrompt("setp ns kUChar UChar B","'B' stored as a UChar (1 Bytes) in ns/kUChar");
  allPass &= testPrompt("setp ns kInt8 Int8 \"-8\"","'-8' stored as a Int8 (1 Bytes) in ns/kInt8");
  allPass &= testPrompt("setp ns kUInt8 UInt8 8","'8' stored as a UInt8 (1 Bytes) in ns/kUInt8");
  allPass &= testPrompt("setp ns kInt16 Int16 \"-32000\"","'-32000' stored as a Int16 (2 Bytes) in ns/kInt16");
  allPass &= testPrompt("setp ns kUInt16 UInt16 50000","'50000' stored as a UInt16 (2 Bytes) in ns/kUInt16");
  allPass &= testPrompt("setp ns kInt32 Int32 \"-2000000000\"","'-2000000000' stored as a Int32 (4 Bytes) in ns/kInt32");
  //allPass &= testPrompt("setp ns kUInt32 UInt32 4000000000","'4000000000' stored as a UInt32 (4 Bytes) in ns/kUInt32");
    allPass &= testPrompt("setp ns kUInt32 UInt32 4000000000","ERROR: 'UInt32' is not a supported data type");
  //allPass &= testPrompt("setp ns kInt64 Int64 \"-9000000000000000000\"","'-9000000000000000000' stored as a Int32 (8 Bytes) in ns/kInt64");
    allPass &= testPrompt("setp ns kInt64 Int64 \"-9000000000000000000\"","ERROR: 'Int64' is not a supported data type");
  //allPass &= testPrompt("setp ns kUInt64 UInt64 15000000000000000000","'15000000000000000000' stored as a UInt32 (8 Bytes) in ns/kUInt64");
    allPass &= testPrompt("setp ns kUInt64 UInt64 15000000000000000000","ERROR: 'UInt64' is not a supported data type");
  allPass &= testPrompt("setp ns kFloat Float 1.2345","'1.2345' stored as a Float (4 Bytes) in ns/kFloat");
  allPass &= testPrompt("setp ns kDouble Double 6.7890","'6.7890' stored as a Double (8 Bytes) in ns/kDouble");
  allPass &= testPrompt("setp ns kString String \"Hello World\"","'Hello World' stored as a String (11 Bytes) in ns/kString");
  allPass &= testPrompt("setp ns kBytes Bytes 1234567890ABCDEF","'1234567890ABCDEF' stored as a Bytes (8 Bytes) in ns/kBytes");

  //test getting type of every data type
  allPass &= testPrompt("getp ns kChar", "Int8/Char");
  allPass &= testPrompt("getp ns kUChar", "UInt8/UChar/Bool");
  allPass &= testPrompt("getp ns kInt8", "Int8/Char");
  allPass &= testPrompt("getp ns kUInt8", "UInt8/UChar/Bool");
  allPass &= testPrompt("getp ns kInt16", "Int16");
  allPass &= testPrompt("getp ns kUInt16", "UInt16");
  allPass &= testPrompt("getp ns kInt32", "Int32");
  //allPass &= testPrompt("getp ns kUInt32", "UInt32");
    allPass &= testPrompt("getp ns kUInt32", "ERROR: unable to locate key 'kUInt32' in namespace 'ns'");
  //allPass &= testPrompt("getp ns kInt64", "Int64");
    allPass &= testPrompt("getp ns kInt64", "ERROR: unable to locate key 'kInt64' in namespace 'ns'");
  //allPass &= testPrompt("getp ns kUInt64", "UInt64");
    allPass &= testPrompt("getp ns kUInt64", "ERROR: unable to locate key 'kUInt64' in namespace 'ns'");
  allPass &= testPrompt("getp ns kFloat", "Float/Double/Bytes (4 Bytes)");
  allPass &= testPrompt("getp ns kDouble", "Float/Double/Bytes (8 Bytes)");
  allPass &= testPrompt("getp ns kString", "String");
  allPass &= testPrompt("getp ns kBytes", "Float/Double/Bytes (8 Bytes)");

  //test retreiving every data type
  allPass &= testPrompt("getp ns kChar Char", "a");
  allPass &= testPrompt("getp ns kUChar UChar", "B");
  allPass &= testPrompt("getp ns kInt8 Int8", "-8");
  allPass &= testPrompt("getp ns kUInt8 UInt8", "8");
  allPass &= testPrompt("getp ns kInt16 Int16", "-32000");
  allPass &= testPrompt("getp ns kUInt16 UInt16", "50000");
  allPass &= testPrompt("getp ns kInt32 Int32", "-2000000000");
  //allPass &= testPrompt("getp ns kUInt32 UInt32", "4000000000");
    allPass &= testPrompt("getp ns kUInt32 UInt32", "ERROR: unable to locate key 'kUInt32' in namespace 'ns'");
  //allPass &= testPrompt("getp ns kInt64 Int64", "-9000000000000000000");
    allPass &= testPrompt("getp ns kInt64 Int64", "ERROR: unable to locate key 'kInt64' in namespace 'ns'");
  //allPass &= testPrompt("getp ns kUInt64 UInt64", "15000000000000000000");
    allPass &= testPrompt("getp ns kUInt64 UInt64", "ERROR: unable to locate key 'kUInt64' in namespace 'ns'");
  allPass &= testPrompt("getp ns kFloat Float", "1.234500");
  allPass &= testPrompt("getp ns kDouble Double", "6.789000");
  allPass &= testPrompt("getp ns kString String", "Hello World");
  allPass &= testPrompt("getp ns kBytes Bytes", "1234567890ABCDEF");

  //test clearing an entire namespace
  allPass &= testPrompt("clearp ns","namespace 'ns' has been cleared");
  allPass &= testPrompt("getp ns kChar", "ERROR: unable to locate key 'kChar' in namespace 'ns'");
  allPass &= testPrompt("getp ns kBytes", "ERROR: unable to locate key 'kBytes' in namespace 'ns'");

  return allPass;
}

bool testPrompt(String prompt, String expectedResponse)
{
  testStream.flush();
  while(cli.errored()) cli.getError();
  while(cli.available()) cli.getCommand();

  cli.parse(prompt);
  if(cli.errored())
  {
    CommandError commandError = cli.getError();
    if(expectedResponse.startsWith("ERROR")) return true;
    Serial.printf("FAIL: error for prompt '%s':\r\n\t%s\r\n",prompt.c_str(),commandError.getMessage().c_str());
    return false;
  }
  if(!cli.available())
  {
    Serial.printf("FAIL: unable to parse prompt '%s'\r\n",prompt.c_str());
    return false;
  }
  Command command = cli.getCommand();
  bool handled = prefCli.handleCommand(command, testStream);
  if(!handled)
  {
    Serial.printf("FAIL: unable to handle prompt '%s'\r\n",prompt.c_str());
    return false;
  }
  String response = testStream.readString();
  response.trim();
  if(!response.equals(expectedResponse))
  {
    Serial.printf("FAIL: incorrect response from prompt '%s':\r\n\tIS: \"%s\"\r\n\tSB: \"%s\"\r\n",prompt.c_str(),response.c_str(),expectedResponse.c_str());
    return false;
  }
  return true;
}
