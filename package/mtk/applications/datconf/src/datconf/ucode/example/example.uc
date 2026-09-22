import * as datconf from 'datconf';

// open file
// or you can use l1parser to get path, build in first
let ctx = datconf.open("/etc/wireless/mediatek/mt7981.dbdc.b0.dat");

if (!ctx) {
    print("Error: " + datconf.error() + "\n");
    exit(1);
}

// read value of key
let ssid = ctx.get("SSID1");
print("Current SSID: " + ssid + "\n");

// set value of key
ctx.set("SSID1", "New_WiFi_Name");
ctx.set("Channel", "36");

// merge key-value pairs with batch
ctx.merge({
    "TxPower": "100",
    "CountryCode": "CN"
});

// get values of all keys
let all = ctx.getall();
for (let k in all) {
    // print(k + "=" + all[k] + "\n");
}

// commit changes and close
ctx.commit();
ctx.close();

// --- utils ---

// parse values, eg: "WPA2;WPA2;WPA2"
let val = "AuthMode=WPA2;WPA2;WPA2";
let second_auth = datconf.get_indexed_value(val, 1); // index from 0 !!

// parse raw strings
let raw_obj = datconf.parse("KEY1=Val1\nKEY2=Val2");