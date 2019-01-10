# Lichtschranke_NANO

Hello everybody,

this is my first approach to a GitHub Project. :smile:
I'm building the code in c/c++ for a hardware that consists of:

- 1x ARDUINO NANO AT-MEGA-328
- 4x 7-Segment-Displays 8 Digits driven with MAX7219-IC
- 1x Encoder with Button
- 1x (currently on the way) Light Barrier made of Laser Diode and Photo Resistor

for photos please check my website:
http://michaelfunk-engineering.de/elektro/lichtschranke.html

What does the Software contain ALREADY:
- Display functions for:
  - general settings of the Displays
  - "strings" >> char[]* >> means you can show words
  - numbers >> to show a whole :dec:-Number from -9999 9999 to 9999 9999
  - time (in ms) >> uses the millis() to show time in seconds.milliseconds

  - getEncoderValue (int) >> encoderValue increments globally

next steps for the software:
- read buttonhit on encoder with interrupt
- displayTimeMinute(...) >> calculation of minutes from milliseconds
                            formating into minutes.seconds.milliseconds
- general MainProgram with following structure:
  1. Set low Lightlevel -> confirm with check on Encoder
  2. Set High lightlevel -> confirm with check on encoder
  3. choose Laps -> with Encoder - confirm
  4. choose How many Players - with Encoder
  5. choose Name Player 1 …
  6. choose Name Player n
  7. Start Timer with encoder
  8. set startTime when barrier crossed
  9. set stopTime when barrier crossed
    - lapTime = stopTime - startTime
    - if (Laps left)
      - set startTime for Lap_n
    - else next Player
  10. Show PLayerData - Select with Encoder (N+1 Sites) - EXIT/RESTART on additional Site


Thanks for interests and help!

Michael Funk :smile:
