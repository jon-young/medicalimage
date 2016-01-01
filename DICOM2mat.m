function [img, slice2name] = DICOM2mat(dicomDir)
%
% Iteratively read in a series of DICOM images into .mat file
% 
% Created: 22 December 2015
% 

cd(dicomDir)
files = dir('*.dcm');
Z = length(files);

% Associate slice number with filename
slice2name = cell(Z,2);
for k = 1:Z
    info = dicominfo(files(k).name);
    slice2name{info.InstanceNumber,1} = info.InstanceNumber;
    slice2name{info.InstanceNumber,2} = files(k).name;
end

% Loop through slices in order and save to array
img = zeros(info.Height, info.Width, Z, 'uint8');
for k = 1:Z
    img(:,:,k) = dicomread(slice2name{k,2});
end
