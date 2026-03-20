import os
from PIL import Image

def resize_badge(base_name):
    for suffix in ['-uhd.png', '-hd.png', '.png']:
        path = os.path.join('resources', f'{base_name}{suffix}')
        if os.path.exists(path):
            img = Image.open(path)
            new_img = img.resize((img.width // 2, img.height // 2), Image.Resampling.LANCZOS)
            new_img.save(path)
            print(f"Resized {path} to {new_img.width}x{new_img.height}")

resize_badge('adminbadge')
resize_badge('modbadge')

print("All badges have been scaled down by 50%.")
