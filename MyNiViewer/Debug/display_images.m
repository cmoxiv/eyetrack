function [rgb depth folder] = display_images(i)

% Those images are stored as raw data, meaning no header.
% The actual format depends on the configuration being used, but if you 
% haven't changed it, then the color image is saved as 3 bytes per pixel 
%(Red, Green, Blue), and the depth map is stored as 16-bit values 
%(2 bytes per pixel) representing depth in millimeters.

image_xRes = 1280;
image_yRes = 1024;
depth_xRes = 640;
depth_yRes = 480;
folder = 'brian_natural';
str = sprintf('%s\\Image_%d.raw',folder, i);
    try
        fp=fopen(str, 'rb');
        d = fread(fp, prod([image_xRes image_yRes 3]));
        red = d(1:3:end);
        green = d(2:3:end);
        blue = d(3:3:end);
        red = reshape(red,[image_xRes image_yRes]);
        blue = reshape(blue,[image_xRes image_yRes]);
        green = reshape(green,[image_xRes image_yRes]);
        red = red./255;
        blue = blue./255;
        green = green./255;

        im = zeros(image_xRes, image_yRes, 3);
        im(:,:,1) = red;
        im(:,:,2) = green;
        im(:,:,3) = blue;
        rgb = zeros(image_yRes, image_xRes, 3);
        rgb(:,:,1) = im(:,:,1)';
        rgb(:,:,2) = im(:,:,2)';
        rgb(:,:,3) = im(:,:,3)';
%         imwrite(rgb, sprintf('%s\\Image_%d.png',folder, i), 'png');
    %     subplot(1,2,1);
    %     imshow(rgb);
        fclose(fp);
    catch exception
        rgb = [];
    end    
    
    % Read Depth images
    str = sprintf('%s\\Depth_%d.raw',folder, i);
    try
        fp=fopen(str, 'rb');
        d = fread(fp, prod([depth_xRes depth_yRes 2]));
        depth = bitor(bitshift(bitor(uint16(0),uint16(d(2:2:end))),8),uint16(d(1:2:end)));
        depth = reshape(depth,depth_xRes,depth_yRes);
        depth = fliplr(imrotate(depth,-90));
    %     subplot(1,2,2);
    %     imagesc(depth);
        fclose(fp);
    catch exception
        depth = [];
    end