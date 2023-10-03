/*
   Copyright (c) 2021 Burks Builds
   This software is licensed under the MIT License. See the license file for details.
   Source: https://github.com/burksbuilds/PreferencesCLI
 */
 
#include "PreferencesCLI.h"
#ifdef ESP32
  #include <nvs_flash.h>
#endif
extern "C" {
  #include "c/cmd.h"
}

PreferencesCLI::PreferencesCLI(Preferences &preferences) : _preferences(preferences)
{
}

void PreferencesCLI::registerCommands(SimpleCLI &cli)
{
	Command setPreferenceCommand = cli.addCommand("setP/ref/erence,setp");
	setPreferenceCommand.setDescription("Used to write the value of entries using the built-in 'Preferences' library.\r\n\tnamespace (-ns): the namespace of the preference setting\r\n\tkey (-k): the string identifier of the preference\r\n\ttype (-t): the type of the data to write (Bool, U/Char, U/Int8, U/Int16, U/Int32, U/Int64, String, Float, Double, Bytes)\r\n\tvalue (-v): the value to store (Bytes parsed as HEX)");
	setPreferenceCommand.addPositionalArgument("namespace,ns");
	setPreferenceCommand.addPositionalArgument("k/ey");
	setPreferenceCommand.addPositionalArgument("t/ype");
	setPreferenceCommand.addPositionalArgument("v/al/ue");

	Command getPreferenceCommand = cli.addCommand("getP/ref/erence,getp");
	getPreferenceCommand.setDescription("Used to read the value of entries using the built-in 'Preferences' library.\r\n\tnamespace (-ns): the namespace of the preference setting\r\n\tkey (-k): the string identifier of the preference\r\n\ttype (-t): the type of the data to read (Bool, U/Char, U/Int8, U/Int16, U/Int32, U/Int64, String, Float, Double, Bytes) (omit to return the type itself)");
	getPreferenceCommand.addPositionalArgument("namespace,ns");
	getPreferenceCommand.addPositionalArgument("k/ey");
	getPreferenceCommand.addPositionalArgument("t/ype","");

	Command clearPreferenceCommand = cli.addCommand("clearP/ref/erence,clearp");
	clearPreferenceCommand.setDescription("Used to clear individual preferences, entire namespaces, or the entire nvs storage area.\r\n\tnamespace (-ns): the namespace of the preference setting (omit to clear entire NVS area)\r\n\tkey (-k): the string identifier of the preference (omit to clear entire namespace)");
	clearPreferenceCommand.addPositionalArgument("namespace,ns","");
	clearPreferenceCommand.addPositionalArgument("k/ey","");
  
}

bool PreferencesCLI::handleCommand(Command &command, Stream &outputStream)
{
  if(cmd_name_equals(command.getPtr(),"setp",4,command.getPtr()->case_sensetive) == CMD_NAME_EQUALS) 
  {
    this->setPreferenceCallback(command.getPtr(), outputStream);
    return true;
  }
  if(cmd_name_equals(command.getPtr(),"getp",4,command.getPtr()->case_sensetive) == CMD_NAME_EQUALS) 
  {
    this->getPreferenceCallback(command.getPtr(), outputStream);
    return true;
  }
  if(cmd_name_equals(command.getPtr(),"clearp",6,command.getPtr()->case_sensetive) == CMD_NAME_EQUALS) 
  {
    this->clearPreferenceCallback(command.getPtr(), outputStream);
    return true;
  }

  return false;
}

void PreferencesCLI::getPreferenceCallback(cmd* commandPointer, Stream &outputStream)
{
  Command cmd(commandPointer);
  Argument name = cmd.getArg("namespace");
  Argument key = cmd.getArg("key");
  Argument type = cmd.getArg("type");
  Argument value = cmd.getArg("value");

  _preferences.begin(name.getValue().c_str());

  String typeString = type.getValue();
  
  //case where key does not exist;
  if(!_preferences.isKey(key.getValue().c_str()))
  {
    sprintf(streamResponseBuffer,"ERROR: unable to locate key '%s' in namespace '%s'",key.getValue().c_str(),name.getValue().c_str());
  } 
  //case where type (not value) is requested
  else if(typeString.length() == 0)
  {
	  PreferenceType pType = _preferences.getType(key.getValue().c_str());
	  const char* typeName = getPreferenceTypeName(pType);
    if (pType == PT_BLOB){
	    size_t byteLength = _preferences.getBytesLength(key.getValue().c_str());
	    sprintf(streamResponseBuffer,"%s (%d Bytes)",typeName,byteLength);  
    } else {
	    sprintf(streamResponseBuffer,"%s",typeName);  
    }
  }
  //normal case, where we get the value and put it in the stream response buffer
  else 
  {
	  PreferenceType pTypeStored = _preferences.getType(key.getValue().c_str());
	  PreferenceType pTypeRequested = getPreferenceType(typeString.c_str());
	  if(pTypeStored != pTypeRequested)
	  {
		  sprintf(streamResponseBuffer,"ERROR: '%s/%s' is stored as a %s, but %s was requested",name.getValue().c_str(),key.getValue().c_str(),getPreferenceTypeName(pTypeStored),getPreferenceTypeName(pTypeRequested));
	  } else {	  
      if(typeString.equalsIgnoreCase("Int8")){
        sprintf(streamResponseBuffer,"%d",_preferences.getChar(key.getValue().c_str()));
      } else if(typeString.equalsIgnoreCase("Char")){
        sprintf(streamResponseBuffer,"%c",_preferences.getChar(key.getValue().c_str())); 
      } else if(typeString.equalsIgnoreCase("UInt8")){
        sprintf(streamResponseBuffer,"%d",_preferences.getUChar(key.getValue().c_str())); 
      } else if(typeString.equalsIgnoreCase("UChar")){
        sprintf(streamResponseBuffer,"%c",_preferences.getUChar(key.getValue().c_str())); 
      } else if(typeString.equalsIgnoreCase("Bool")){
        sprintf(streamResponseBuffer,"%s",_preferences.getBool(key.getValue().c_str())?"TRUE":"FALSE");
      } else if(typeString.equalsIgnoreCase("Int16")){
        sprintf(streamResponseBuffer,"%d",_preferences.getShort(key.getValue().c_str())); 
      } else if(typeString.equalsIgnoreCase("UInt16")){
        sprintf(streamResponseBuffer,"%d",_preferences.getUShort(key.getValue().c_str()));
      } else if(typeString.equalsIgnoreCase("Int32")){
        sprintf(streamResponseBuffer,"%d",_preferences.getInt(key.getValue().c_str()));
      } else if(typeString.equalsIgnoreCase("UInt32")){
        sprintf(streamResponseBuffer,"%d",_preferences.getUInt(key.getValue().c_str()));
      } else if(typeString.equalsIgnoreCase("Int64")){
        sprintf(streamResponseBuffer,"%d",_preferences.getLong64(key.getValue().c_str()));
      } else if(typeString.equalsIgnoreCase("UInt64")){
        sprintf(streamResponseBuffer,"%d",_preferences.getULong64(key.getValue().c_str()));
      } else if(typeString.equalsIgnoreCase("Float") ){
        sprintf(streamResponseBuffer,"%f",_preferences.getFloat(key.getValue().c_str()));
      } else if(typeString.equalsIgnoreCase("Double") ){
        sprintf(streamResponseBuffer,"%f",_preferences.getDouble(key.getValue().c_str()));
      } else if(typeString.equalsIgnoreCase("Bytes")){
        size_t dataLength = _preferences.getBytesLength(key.getValue().c_str());
        if(dataLength >= PREFERENCES_CLI_RESPONSE_BUFFER_SIZE/2) dataLength = PREFERENCES_CLI_RESPONSE_BUFFER_SIZE/2-1;
        
        char * data = (char*)malloc(dataLength);
        _preferences.getBytes(key.getValue().c_str(), data, dataLength);
        for(int i=0; i < dataLength; ++i)
        {
          uint8_t mostSig = data[i] / 16;
          uint8_t leastSig = data[i] - 16*mostSig;
          streamResponseBuffer[i*2] = getHexChar(mostSig);
          streamResponseBuffer[i*2+1] = getHexChar(leastSig);
        }
        outputStream.println();
        streamResponseBuffer[dataLength*2] = '\0';
        free(data);
      } else if(typeString.equalsIgnoreCase("String")){
        _preferences.getString(key.getValue().c_str(), (char*)streamResponseBuffer, PREFERENCES_CLI_RESPONSE_BUFFER_SIZE);
      } else {
        sprintf(streamResponseBuffer,"ERROR: unable to get preferences of type '%s'",typeString);
      }
	  }
  }

  _preferences.end(); 
  outputStream.println(streamResponseBuffer);
  
}

void PreferencesCLI::setPreferenceCallback(cmd* commandPointer, Stream &outputStream)
{
	Command cmd(commandPointer);
	Argument name = cmd.getArg("namespace");
	Argument key = cmd.getArg("key");
	Argument type = cmd.getArg("type");
	Argument value = cmd.getArg("value");
	
	
	String typeString = type.getValue();
	PreferenceType prefType = getPreferenceType(typeString.c_str());

	_preferences.begin(name.getValue().c_str());

	size_t bytesWritten = 0;

	if(typeString.equalsIgnoreCase("Int8")){
		bytesWritten = _preferences.putChar(key.getValue().c_str(), (int8_t)(value.getValue().toInt()));
	} else if(typeString.equalsIgnoreCase("Char")){
		bytesWritten = _preferences.putChar(key.getValue().c_str(), value.getValue().charAt(0)); 
	} else if(typeString.equalsIgnoreCase("UInt8")){
		bytesWritten = _preferences.putUChar(key.getValue().c_str(), (uint8_t)(value.getValue().toInt())); 
	} else if(typeString.equalsIgnoreCase("UChar")){
		bytesWritten = _preferences.putUChar(key.getValue().c_str(), (uint8_t)(value.getValue().charAt(0))); 
	} else if(typeString.equalsIgnoreCase("Bool")){
		bytesWritten = _preferences.putBool(key.getValue().c_str(), value.getValue().toInt() != 0 || String(value.getValue().charAt(0)).equalsIgnoreCase("T")); 
	} else if(typeString.equalsIgnoreCase("Int16")){
		bytesWritten = _preferences.putShort(key.getValue().c_str(), (int16_t)(value.getValue().toInt())); 
	} else if(typeString.equalsIgnoreCase("UInt16")){
		bytesWritten = _preferences.putUShort(key.getValue().c_str(), (uint16_t)(value.getValue().toInt())); 
	} else if(typeString.equalsIgnoreCase("Int32")){
		bytesWritten = _preferences.putInt(key.getValue().c_str(), value.getValue().toInt()); 
	//} else if(typeString.equalsIgnoreCase("UInt32")){
		//bytesWritten = _preferences.putUInt(key.getValue().c_str(), value.getValue().toInt()); #TODO: Fix loss of precision...
	//} else if(typeString.equalsIgnoreCase("Int64")){
		//bytesWritten = _preferences.putLong64(key.getValue().c_str(), value.getValue().toInt());  #TODO: Fix loss of precision...
	//} else if(typeString.equalsIgnoreCase("UInt64")){
		//bytesWritten = _preferences.putULong64(key.getValue().c_str(), value.getValue().toInt());  #TODO: Fix loss of precision...
	} else if(typeString.equalsIgnoreCase("Float") ){
		bytesWritten = _preferences.putFloat(key.getValue().c_str(), value.getValue().toFloat()); 
	} else if(typeString.equalsIgnoreCase("Double")){
		bytesWritten = _preferences.putDouble(key.getValue().c_str(), value.getValue().toDouble()); 
	} else if(typeString.equalsIgnoreCase("Bytes") ) 
	{
		String valueText = value.getValue();
		const char * valueTextPtr = valueText.c_str();
		size_t dataLength = (valueText.length())/2;//ignores final char in odd-lengthed hex strings
		uint8_t * data = (uint8_t*)malloc(dataLength);
		for(int i=0; i < dataLength; ++i)
		{
		  data[i] = 16*parseHexChar(valueTextPtr[i*2]) + parseHexChar(valueTextPtr[i*2+1]);
		}
		bytesWritten = _preferences.putBytes(key.getValue().c_str(), data, dataLength);
		free(data);
	} 
	else if(typeString.equalsIgnoreCase("String")){
		bytesWritten = _preferences.putString(key.getValue().c_str(), value.getValue()); 
	} else{
    sprintf(streamResponseBuffer,"ERROR: '%s' is not a supported data type",typeString.c_str());
    _preferences.end(); 
	  outputStream.println(streamResponseBuffer);
    return;
  } 


	if (bytesWritten > 0){
		sprintf(streamResponseBuffer,"'%s' stored as a %s (%d Bytes) in %s/%s",value.getValue().c_str(),typeString,bytesWritten,name.getValue().c_str(), key.getValue().c_str());
	} else {
		sprintf(streamResponseBuffer,"ERROR: unable to store '%s' as a %s in %s/%s",value.getValue().c_str(),typeString.c_str(),name.getValue().c_str(), key.getValue().c_str());
	}

	_preferences.end(); 
	outputStream.println(streamResponseBuffer);
}

void PreferencesCLI::clearPreferenceCallback(cmd* commandPointer, Stream &outputStream)
{
  Command cmd(commandPointer);
  Argument name = cmd.getArg("namespace");
  Argument key = cmd.getArg("key");

  if(key.isSet() && name.isSet())
  {
    if(!_preferences.begin(name.getValue().c_str()))
    {
      sprintf(streamResponseBuffer,"ERROR: unable to open namespace '%s'",name.getValue());
    } else 
    {
      if(_preferences.isKey(key.getValue().c_str()))
      {
        if(!_preferences.remove(key.getValue().c_str()))
        {
          sprintf(streamResponseBuffer,"ERROR: unable to clear '%s' from namespace '%s'",key.getValue(),name.getValue());
        } else {
		  sprintf(streamResponseBuffer,"'%s' has been cleared from namespace '%s'",key.getValue(),name.getValue());
		}
      } else {
	    sprintf(streamResponseBuffer,"'%s' was not a preference in namespace '%s'",key.getValue(),name.getValue());
	  }
      _preferences.end();
    }
  } else if (name.isSet())
  {
    if(!_preferences.begin(name.getValue().c_str()))
    {
      sprintf(streamResponseBuffer,"ERROR: unable to open namespace '%s'",name.getValue());
    } else {
      if(!_preferences.clear())
      {
        sprintf(streamResponseBuffer,"ERROR: unable to clear namespace '%s'",name.getValue());
      } else {
	    sprintf(streamResponseBuffer,"namespace '%s' has been cleared",name.getValue());
	  }
      _preferences.end();
    }
  } else {
    #ifdef ESP32
		nvs_flash_erase();
		nvs_flash_init();
		sprintf(streamResponseBuffer,"All preferences have been cleared from the NVS flash");
	#else
		sprintf(streamResponseBuffer,"ERROR: full clearing of all preferences is only supported on the ESP32");
	#endif
  }
  outputStream.println(streamResponseBuffer);
}

const char* PreferencesCLI::getPreferenceTypeName(PreferenceType pType)
{
	switch(pType)
	{
		case PT_I8: return "Int8/Char";
		case PT_U8: return "UInt8/UChar/Bool";
		case PT_I16: return "Int16";
		case PT_U16: return "UInt16";
		case PT_I32: return "Int32";
		case PT_U32: return "UInt32";
		case PT_I64: return "Int64";
		case PT_U64: return "UInt64";
		case PT_STR: return "String";
		case PT_BLOB: return "Float/Double/Bytes";
		default: return "Invalid";
	}
}

PreferenceType PreferencesCLI::getPreferenceType(const char* typeName)
{
	String typeString = String(typeName);
	if(typeString.equalsIgnoreCase("Int8")){
		return PT_I8;
	} else if(typeString.equalsIgnoreCase("Char")){
		return PT_I8;
	} else if(typeString.equalsIgnoreCase("UInt8")){
		return PT_U8;
	} else if(typeString.equalsIgnoreCase("UChar")){
		return PT_U8;
	} else if(typeString.equalsIgnoreCase("Bool")){
		return PT_U8;
	} else if(typeString.equalsIgnoreCase("Int16")){
		return PT_I16;
	} else if(typeString.equalsIgnoreCase("UInt16")){
		return PT_U16;
	} else if(typeString.equalsIgnoreCase("Int32")){
		return PT_I32;
	} else if(typeString.equalsIgnoreCase("UInt32")){
		return PT_U32;
	} else if(typeString.equalsIgnoreCase("Int64")){
		return PT_I64;
	} else if(typeString.equalsIgnoreCase("UInt64")){
		return PT_U64;
	} else if(typeString.equalsIgnoreCase("Float")){
		return PT_BLOB;
	} else if(typeString.equalsIgnoreCase("Double")){
		return PT_BLOB;
	} else if(typeString.equalsIgnoreCase("Bytes")){
		return PT_BLOB;
	} else if(typeString.equalsIgnoreCase("String")){
		return PT_STR;
	} else {
		return PT_INVALID;
	}
}


uint8_t PreferencesCLI::parseHexChar(char hex)
{
	if(hex >= '0' && hex <= '9')
	{
		return hex - '0';
	} else if(hex >= 'A' && hex <= 'F')
	{
		return hex - 'A' + 10;
	} else if(hex >= 'a' && hex <= 'f')
	{
		return hex - 'a';
	} else {
		return 0;
	}
	
}

char PreferencesCLI::getHexChar(uint8_t val)
{
	if(val < 10)
	{
		return '0'+val;
	} else if(val < 16)
	{
		return 'A'+(val-10);
	} else {
		return ' ';
	}
}