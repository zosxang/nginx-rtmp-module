
## AD Insertion markers

nginx-rtmp module can interpret ad insertion markers message sent by encoder in amf rmtp message.
It then extract the information and write it in an emsg box at the begining of the next chunk.
The emsg box contains the relative start time of the ad-break, its duration, and possibly some metadata.
Enabling ad_marker also add the following inband event in the manifest.
This is the responsability of the player to watch this event, and to treat it.

```
<InbandEventStream schemeIdUri="urn:scte:scte35:2013:xml" value="1" />
```

Options to enable ad marker processing are :

```
dash_ad_markers off|on_cuepoint|on_cuepoint_scte35;
```

 - on_cuepoint is the simple variant (without scte message) 
 - on_cuepoint_scte35 is the scte35 variant with program_id metadata 

```
dash_ad_markers_timehack off|on;
```

 - off implement the standard timing as described in reference documentation
 - on implement a hack on the start time. This is need to be more resilient. Warning this need a patched version of your player. (currently this is what is test and in production with a  one line patch on dashjs)

Currently there is only elemental encoder tested and compliant.

See [here](https://theyosh.nl/speeltuin/dash/dash.js-2.0.0/samples/ad-insertion/) for the original examples and inspiration.
