#!/usr/bin/env python3

from scipy.io import wavfile

samplerate, data = wavfile.read('./bad-apple.wav')

print(f"{samplerate = }")
print(f"{data.shape = }")
