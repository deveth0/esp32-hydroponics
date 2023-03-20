#ifndef HYDROPONICS_CFG_H
#define HYDROPONICS_CFG_H

bool deserializeConfig(JsonObject doc, bool fromFS = false);
void deserializeConfigFromFS();
bool deserializeConfigSec();
void serializeConfig();
void serializeConfigSec();


#endif