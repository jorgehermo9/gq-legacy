# Where should we write the GIF?
Output demo.gif

# Set up a 1200x600 terminal with 46px font.
Set FontSize 46
Set Width 1450
Set Height 1300
Set PlaybackSpeed 0.7
Set Padding 50

Type "bat example.json"

Sleep 500ms

Enter

Sleep 4s

Ctrl+L

Sleep 500ms

Type "bat query.gq"

Sleep 500ms

Enter

Sleep 4s

Ctrl+L

Sleep 500ms

Type "gq query.gq --json example.json | bat -l json"

Sleep 500ms

Enter

Sleep 8s
