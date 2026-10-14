# Optional Analog Inputs

Enable the known four analog input PDOs with a local checkout:

```yaml
packages:
  analog_inputs: !include packages/analog_inputs.yml
```

The package extends the CAN bus selected by the `comfoair_canbus_id`
substitution, defaulting to `comfoair_canbus`. It adds only read requests and
diagnostic sensor entities; there are no setting/control actions.

PDOs 369-372 are UINT8, exactly one byte, according to the protocol documentation
and aiocomfoconnect registry. The package validates that width before reading the
byte and scales by 0.1 V, matching aiocomfoconnect. Empty or wrong-length frames
do not replace a previous reading. No response remains unknown; zero is not
synthesized when an input is missing.

Whether a voltage comes from a CO2 sensor, potentiometer, or another source
depends on the installation/OptionBox. Voltage-to-ppm conversion is sensor
specific and is not guessed by this package.

Q600 normal-condition reads return zero for these channels; that validates
transport/decoding, not calibrated nonzero voltage or an installed OptionBox.
The host tests cover nonzero conversion (75 -> 7.5 V) and wrong-width rejection.
An OptionBox/potentiometer or known-voltage test is still needed for physical
nonzero verification.

References:
- https://github.com/michaelarnauts/aiocomfoconnect/blob/8fe395b3f8657e25ef8c5a6ccea5aab06808a514/docs/PROTOCOL-PDO.md
- https://github.com/michaelarnauts/aiocomfoconnect/blob/8fe395b3f8657e25ef8c5a6ccea5aab06808a514/aiocomfoconnect/sensors.py
- https://github.com/yoziru/esphome-zehnder-comfoair/issues/33
