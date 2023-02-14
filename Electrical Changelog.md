# Electrical Changelog

week 1 and 2 were mainly research and PDR prep.

### types of changes: 

    - **Actions** what was done  and result
    - **Issues** things to address / troubleshoot
    - **TODO** next steps

newest changes are first
last updated: 2/14/2023

---


## week 5

Troubleshooting part 2 the electric boogaloo

### Actions
- ask lol 

### Issues

- board 0 (first 1, not CAN reciever board) bridge is bad
    - has 5V straight to ardinuo (max is 3.3V) 
    - rotational switch doesnt change the voltage as it should

### TODO

- [ ] measure PCBs on old table to verify measurements for redesign
- [ ] look into premade 3.3V <-> 5V converter (and not homemade resistor bridges) -- Dr. Conner

---

## week 4

troubleshooting CAN <br>

### Actions

- resoldered bad connecctions on board 3
- LOW CAN value measures as 1.6V (ish). HIGH CAN value okay
- PCB redisign in KiCAD

### Issues

- boards reading *state 8* until Arduino reset 
- goal sensor doesnt send message to touch screen (no print statement)

### TODO

- [ ] research how to fix LOW CAN value
- [ ] print prototype PCB board with fixes

---

## week 3

Troubleshootung CAN system

### Actions

- read serial output from motor contoller PCBS

### Issues

- board 3 has bad solder connections
- boards get stuck in *state 8* print loop

### TODO

- [x] resolder bad connections on boad 3
- [ ] research why boards stuck in *state 8*

---
