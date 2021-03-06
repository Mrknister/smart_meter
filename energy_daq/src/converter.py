#!/usr/bin/env python3

# The MIT License (MIT)
#
# Copyright (c) 2016 Thomas Kriechbaumer
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import h5py
import numpy
import os
import re
import sys
import time
import traceback

DESTINATION = 'files/ram'
CALIBRATION_FACTOR_ATTR = 'calibration_factor'
REMOVED_OFFSET_ATTR = 'removed_offset'


class MicroControllerDevice(object):
    """
    Packet Layout:
        bytes 0:  MSB trigger number
        bytes 1:  LSB trigger number
        bytes 2:  Sample D11
        bytes 3:  Sample D10
        bytes 4:  Sample D9
        bytes 5:  Sample D8
        bytes 6:  Sample D7
        bytes 7:  Sample D6
        bytes 8:  Sample D5
        bytes 9:  Sample D4
        bytes 10: Sample D3
        bytes 11: Sample D2
        bytes 12: Sample D1
        bytes 13: Sample D0
    """

    CALIBRATION_5A = 0.005405405  # 1 / 0.185 V/A * (4.096 / 4096)
    CALIBRATION_30A = 0.015151515  # 1 / 0.066 V/A * (4.096 / 4096)
    CALIBRATION_VOLTAGE = 0.2853  # 230V / (6V * 1.478 estimated IdleVolt) * (100000Ohm + 10000Ohm) / 10000Ohm) * (4.096 / 4096)

    CALIBRATION_CURRENT = {
        1: CALIBRATION_30A,
        2: CALIBRATION_5A,
        3: CALIBRATION_5A,
        4: CALIBRATION_5A,
        5: CALIBRATION_5A,
        6: CALIBRATION_5A,
    }

    def read_data(self, raw_data):
        dtypes = [
            ('trigger', '>u2'),
            ('D11', 'B'),
            ('D10', 'B'),
            ('D9', 'B'),
            ('D8', 'B'),
            ('D7', 'B'),
            ('D6', 'B'),
            ('D5', 'B'),
            ('D4', 'B'),
            ('D3', 'B'),
            ('D2', 'B'),
            ('D1', 'B'),
            ('D0', 'B'),
        ]
        return numpy.frombuffer(raw_data, dtype=dtypes)

    def parse_channels(self, data, output_file, **default_dataset_options):
        # channel 0 is empty and free-floating

        for channel_id in [1, 2, 3, 4, 5, 6]:
            name = 'current{}'.format(channel_id)
            dset = output_file.create_dataset(name, **default_dataset_options)
            self._parse_current(data, dset, channel_id)

        name = 'voltage'
        dset = output_file.create_dataset(name, **default_dataset_options)
        self._parse_voltage(data, dset, 7)

    def _parse_voltage(self, data, dset, channel_id):
        dset[:] = self._extract_channel(data, channel_id)
        mean_voltage_offset = numpy.mean(dset[:])
        dset[:] -= int(mean_voltage_offset)
        dset.attrs.create(CALIBRATION_FACTOR_ATTR, self.CALIBRATION_VOLTAGE, dtype='f8')
        dset.attrs.create(REMOVED_OFFSET_ATTR, mean_voltage_offset, dtype='f8')

    def _parse_current(self, data, dset, channel_id):
        dset[:] = self._extract_channel(data, channel_id)
        dset[:] -= 2500
        dset.attrs.create(CALIBRATION_FACTOR_ATTR, self.CALIBRATION_CURRENT[channel_id], dtype='f8')
        dset.attrs.create(REMOVED_OFFSET_ATTR, 2500, dtype='uint16')

    def _extract_channel(self, data, channel_id):
        result = data.view('B').reshape(len(data), 14)[:, 2:14].astype('u2')
        result &= 1 << channel_id
        result >>= channel_id
        result *= numpy.array([1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048], dtype='u2')
        return numpy.sum(result, axis=1)


class FPGADevice(object):
    """
    Packet Layout:
        bytes 0:  trigger tag LSB
        bytes 1:  trigger tag MSB
        bytes 2:  ADC0 LSB
        bytes 3:  ADC0 MSB
        bytes 4:  ADC1 LSB
        bytes 5:  ADC1 MSB
        bytes 6:  ADC2 LSB
        bytes 7:  ADC2 MSB
        bytes 8:  ADC3 LSB
        bytes 9:  ADC3 MSB
        bytes 10: ADC4 LSB
        bytes 11: ADC4 MSB
        bytes 12: ADC5 LSB
        bytes 13: ADC5 MSB
    """

    CALIBRATION_VOLTAGE = 0.011170775  # 225Vrms => 317.25Vpeak == 28400 => 28400 / 317.25
    CALIBRATION_CURRENT = 0.000962500  # 3 turns, 50Arms, calibrated on 2016-09-14 with kettle: 7.15Arms + 0.19Arms baseline

    def read_data(self, raw_data):
        dtypes = [
            ('trigger', '<u2'),
            ('voltage1', '<i2'), ('current1', '<i2'),
            ('voltage2', '<i2'), ('current2', '<i2'),
            ('voltage3', '<i2'), ('current3', '<i2'),
        ]
        return numpy.frombuffer(raw_data, dtype=dtypes)

    def parse_channels(self, data, output_file, **default_dataset_options):
        for phase_id in [1, 2, 3]:
            name = 'voltage{}'.format(phase_id)
            dset = output_file.create_dataset(name, data=data[name], **default_dataset_options)
            dset.attrs.create(CALIBRATION_FACTOR_ATTR, self.CALIBRATION_VOLTAGE, dtype='f8')

            name = 'current{}'.format(phase_id)
            dset = output_file.create_dataset(name, data=data[name], **default_dataset_options)
            dset.attrs.create(CALIBRATION_FACTOR_ATTR, self.CALIBRATION_CURRENT, dtype='f8')


class Converter(object):

    def __init__(self, input_file, frequency, device='microcontroller', keep_file=False, gnuplot=False):
        self.input_file = input_file
        self.frequency = frequency
        self.device = MicroControllerDevice() if device == 'microcontroller' else FPGADevice()
        self.keep_file = keep_file
        self.gnuplot = gnuplot

    def start(self):
        start_time = time.time()

        self._read_data()
        self._parse_data()

        if not self.keep_file:
            os.remove(self.input_file)

        os.rename(self.output_file + '.inprogress', self.output_file)

        time_elapsed = time.time() - start_time
        _log('Converted to {} in {} seconds and {} bytes.'.format(
            os.path.basename(self.output_file),
            int(round(time_elapsed)),
            os.path.getsize(self.output_file),
        ))

    def _read_data(self):
        with open(self.input_file, 'rb') as input_file:
            self.filesize = os.path.getsize(self.input_file)
            self.samples_count = int(self.filesize / 14)

            raw_data = input_file.read(self.samples_count * 14)
            self.data = self.device.read_data(raw_data)

    def _parse_data(self):
        try:
            # unit-2016-06-04T22-24-42.411571+0200-0000001.bin
            g = re.match(
                '^'
                '(?P<name>.+)-'
                '(?P<year>\d{4})-(?P<month>\d{2})-(?P<day>\d{2})'
                'T(?P<hours>\d{2})-(?P<minutes>\d{2})-(?P<seconds>\d{2})\.(?P<microseconds>\d+)(?P<timezone>.+)-'
                '(?P<sequence>\d+)'
                '.bin'
                '$',
                os.path.basename(self.input_file)).groupdict()
        except:
            raise ValueError('Filename not matched! Ignoring file: {}'.format(os.path.basename(self.input_file)))

        if self.filesize % 14 != 0:
            _log('Last packet in file incomplete in file: {}'.format(self.input_file))

        file = os.path.basename(self.input_file)
        filename, ext = os.path.splitext(file)
        self.output_file = os.path.join(DESTINATION, filename + '.hdf5')
        with h5py.File(self.output_file + '.inprogress', 'w', driver='core') as output_file:
            output_file.attrs.create('name', bytes(str(g['name']), 'ASCII'))
            output_file.attrs.create('year', int(g['year']), dtype='uint32')
            output_file.attrs.create('month', int(g['month']), dtype='uint32')
            output_file.attrs.create('day', int(g['day']), dtype='uint32')
            output_file.attrs.create('hours', int(g['hours']), dtype='uint32')
            output_file.attrs.create('minutes', int(g['minutes']), dtype='uint32')
            output_file.attrs.create('seconds', int(g['seconds']), dtype='uint32')
            output_file.attrs.create('microseconds', int(g['microseconds']), dtype='uint32')
            output_file.attrs.create('sequence', int(g['sequence']), dtype='uint64')
            output_file.attrs.create('timezone', bytes(str(g['timezone']), 'ASCII'))
            output_file.attrs.create('frequency', self.frequency, dtype='uint64')
            output_file.attrs.create('first_trigger_id', self.data['trigger'][0], dtype='uint16')
            output_file.attrs.create('last_trigger_id', self.data['trigger'][-1], dtype='uint16')

            self.device.parse_channels(self.data,
                                       output_file,
                                       shape=(self.samples_count,),
                                       dtype='<i2',
                                       fletcher32=True,
                                       compression='gzip',
                                       compression_opts=9,
                                       shuffle=True,
                                       )

            if self.gnuplot:
                for i in range(self.samples_count):
                    if isinstance(self.device, MicroControllerDevice):
                        print("{} {} {} {} {} {} {}".format(
                            output_file['/current1'][i],
                            output_file['/current2'][i],
                            output_file['/current3'][i],
                            output_file['/current4'][i],
                            output_file['/current5'][i],
                            output_file['/current6'][i],
                            float(output_file['/voltage'][i])*0.2853
                        ))
                    else:
                        print("{} {} {} {} {} {}".format(
                            output_file['/voltage1'][i],
                            output_file['/current1'][i],
                            output_file['/voltage2'][i],
                            output_file['/current2'][i],
                            output_file['/voltage3'][i],
                            output_file['/current3'][i]
                        ))


def _log(message):
    print(message, file=sys.stderr)

    with open('files/converter.log', 'a') as f:
        timestamp = time.strftime("%Y-%m-%dT%H:%M:%S%z", time.localtime())
        ms = message.split("\n")
        for m in ms:
            f.write("{} {}\n".format(timestamp, m))


def __main__():
    gnuplot = False
    if '--gnuplot' in sys.argv:
        sys.argv.remove('--gnuplot')
        gnuplot = True

    keep_file = False
    if '--keep-file' in sys.argv:
        sys.argv.remove('--keep-file')
        keep_file = True

    if len(sys.argv) != 4:
        print('Invalid arguments. Expected device, filename, and frequency.', file=sys.stderr)
        return

    try:
        Converter(sys.argv[1], sys.argv[2], sys.argv[3], keep_file=keep_file, gnuplot=gnuplot).start()
    except Exception as e:
        _log("Converting failed: {}".format(traceback.format_exc()))

if __name__ == '__main__':
    __main__()
