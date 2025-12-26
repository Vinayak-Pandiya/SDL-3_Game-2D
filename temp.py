# from rembg import remove
# from PIL import Image

# # Load the input image
# input_path = ".\\characters\\Map Data\\Ai genrated\\generated-image2-pro.png"     # change this to your image file
# output_path = "output.png"    # transparent background will be saved as PNG

# # Open the image
# input_image = Image.open(input_path)

# # Remove the background
# output_image = remove(input_image)

# # Save the result
# output_image.save(output_path)

# print(f"Background removed! Saved as {output_path}")

import yt_dlp

url = "https://youtu.be/AsjrajbysXQ?si=FelwDmcldmSVNFhT"

ydl_opts = {
    "format": "bestaudio/best",
    "postprocessors": [{
        "key": "FFmpegExtractAudio",
        "preferredcodec": "mp3",
        "preferredquality": "192",
    }],
}

with yt_dlp.YoutubeDL(ydl_opts) as ydl:
    ydl.download([url])