EESchema Schematic File Version 5
LIBS:off_hook_detection
EELAYER 27 0
EELAYER END
$Descr User 11811 8267
encoding utf-8
Sheet 1 1
Title "Off-Hook Detection Circuit"
Date "2024-02-17"
Rev "1.1"
Comp "ESP32 + Optocoupler-Based Circuit"
Comment1 "Designed for detecting off-hook status in telephone circuits"
Comment2 "Utilizes PC817 for electrical isolation"
$EndDescr

$Comp
L Opto-PC817 U1
U 1 1 60F59E2A
P 4500 3500
F 0 "U1" H 4500 3800 50  0000 C CNN
F 1 "PC817" H 4500 3200 50  0000 C CNN
F 2 "Package_DIP:DIP-4_W7.62mm" H 4500 3100 50  0001 C CNN
F 3 "" H 4500 3500 50  0001 C CNN
	1    4500 3500
	1    0    0    -1  
$EndComp

$Comp
L Device:R R1
U 1 1 60F5A1E8
P 3750 3500
F 0 "R1" V 3650 3500 50  0000 C CNN
F 1 "470Ω 1W" V 3850 3500 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10mm_Horizontal" H 3750 3500 50  0001 C CNN
F 3 "" H 3750 3500 50  0001 C CNN
	1    3750 3500
	0    -1   -1   0   
$EndComp

$Comp
L Device:R R2
U 1 1 60F5A30C
P 5250 3500
F 0 "R2" V 5150 3500 50  0000 C CNN
F 1 "10kΩ" V 5350 3500 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10mm_Horizontal" H 5250 3500 50  0001 C CNN
F 3 "" H 5250 3500 50  0001 C CNN
	1    5250 3500
	0    -1   -1   0   
$EndComp

$Comp
L Device:D_Zener D1
U 1 1 60F5A436
P 5500 3700
F 0 "D1" H 5500 3800 50  0000 C CNN
F 1 "5.1V Zener" H 5500 3600 50  0000 C CNN
F 2 "Diode_THT:D_DO-35_SOD27_P5.08mm_Horizontal" H 5500 3700 50  0001 C CNN
F 3 "" H 5500 3700 50  0001 C CNN
	1    5500 3700
	1    0    0    -1  
$EndComp

Wire Wire Line
	3950 3500 4300 3500
Wire Wire Line
	4700 3500 5100 3500
Wire Wire Line
	5400 3500 5500 3500
Wire Wire Line
	5500 3500 5500 3550
Wire Wire Line
	5500 3850 5500 4000
Wire Wire Line
	5500 4000 4500 4000
Wire Wire Line
	4500 4000 4500 3700

Text Label 3750 3400 0    50   ~ 0
PHONE_TIP

Text Label 3750 3600 0    50   ~ 0
PHONE_RING

Text Label 5250 3400 0    50   ~ 0
ESP32_GPIO

$EndSCHEMATC
