import sys, os, subprocess

file = sys.argv[1]
name = os.path.splitext(file)[0]

channels = subprocess.Popen("identify -format '%[channels]' {file}".format(file=file), shell=True, stdout=subprocess.PIPE)
channels = channels.stdout.read().decode("utf-8").strip()
print(channels)

# This conversion removes alpha channel
os.system('convert -endian LSB {file} /tmp/denoise-in.pfm'.format(file=file))
os.system('oidnDenoise --ldr /tmp/denoise-in.pfm --srgb -o /tmp/denoise-out.pfm')

if channels == 'srgba':
    # Extract alpha channel and denoise it too
    os.system('convert -endian LSB {file} -alpha extract -colorspace RGB -type truecolor /tmp/denoise-in-alpha.pfm'.format(file=file))
    os.system('oidnDenoise --ldr /tmp/denoise-in-alpha.pfm -o /tmp/denoise-out-alpha.pfm')

    os.system('convert /tmp/denoise-out.pfm /tmp/denoise-out-alpha.pfm -compose CopyOpacity -composite {name}.denoised.png'.format(name=name))
else:
    os.system('convert /tmp/denoise-out.pfm {name}.denoised.png'.format(name=name))
