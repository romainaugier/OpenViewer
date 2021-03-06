# [Download the latest version of Filmic Nuke](https://github.com/sobotka/filmic-nuke/archive/refs/heads/main.zip)

# Who?

This is a simple OpenColorIO configuration for intermediate to advanced imagers using Nuke.

# What?

This OpenColorIO configuration adds a closer-to-photorealistic view transform for your renders. For imagers working with non-photorealistic rendering, it also will bring significant dynamic range and lighting capabilities to your work, as well as potentially open up correct transforms for rendering to HDR displays and other such forward looking technology. It is as close to a Magic Button™ you can get for an experienced imager. The kit embodies a high dynamic range transfer function and an intensity gamut mapping.

Filmic does two things:
1. It compresses the scene referred linear radiometric energy values down to the display / output referred range. This aspect is known as a transfer function or tone mapping. The shape of the Filmic Base Log with a contrast aesthetic roughly emulates a photographic film curve.
1. It compresses the gamut for high intensity values. As colour ratios increase in intensity, highly saturated ratios tend to be resistant to transfer function compression, which results in peculiar feeling imagery with some regions feeling appropriately over-exposed and others "lingering" behind. Filmic considers all colour values fair game, and attempts to blend colours into a consistent output that matches our learned expectations from film emulsion-like media.

# When?

This repository is ready to use right now.

# Why?

Because the basic sRGB nonlinear transfer functions (OETF / EOTF) were designed to describe an aspect of device response and never for rendering. This configuration is a step towards providing imagers with a reliable camera rendering transform and a base of aesthetic looks useful for modern raytracing engine CGI, animation, and visual effects work with real-world cameras.

# How?

1. [Download the latest version of Filmic Nuke](https://github.com/sobotka/filmic-nuke/archive/refs/heads/main.zip). Replace your current OpenColorIO configuration via the Nuke Project Settings panel.

# Supported Display Colorimetry

The current configuration supports:
 * Generic sRGB / REC.709 displays with 2.2 native power function EOTF.
 * Generic BT.1886 displays with a 2.4 native power function EOTF.
 * Display P3 displays with 2.2 native power function EOTF. Examples include:
   * Apple MacBook Pros from 2016 on.
   * Apple iMac Pros.
   * Apple iMac from late 2015 on.

# Additional Information and Technical Details

The basic kit of weaponry includes:

## View Transforms

A set of View transforms that include:

 1. ***sRGB OETF***. This is an accurate version of the sRGB transfer function.
 1. ***Display EOTF***. This is the native display EOTF for stimulus linear output.
 1. ***Non-Colour Data***. This is a view useful for evaluating a data format. Do not expect to see perceptual values however, as it is literally data dumped directly to the screen. Use this transform on your buffer, via the *UV Image Viewer* Properties panel, if your buffer represents data and not colour information. This will keep it out of the OpenColorIO transformation pipeline chain and leave it as data.
 1. ***Linear Raw***. This is a colour managed linearized version of your data. For all intents an purposes, will look identical to ***Non-Colour Data***, but applied to colour based data such as an image.
 1. ***Filmic Log Encoding Base***. This is the workhorse View for all of your rendering work. Setting it in the View will result in a log encoded appearance, which will look exceptionally low contrast. Use this if you want to adjust the image for grading using another tool such as Resolve, with no additional modifications. Save to a high bit depth display referred format such as 16 bit TIFF. This basic view is designed to be coupled with one of the contrast looks.

## Look Transforms

A set of Look transforms that include:

 1. ***Greyscale***. This Look is based off of the ***Filmic Log Encoding Base*** and will deliver a weighted greyscale version of the image. The weights used are for REC.709 RGB lights, which are the same lights specified in sRGB.
 1. Five contrast base looks for use with the ***Filmic Log Encoding Base***. All map middle grey 0.18 to 0.5 display referred. Each has a smooth roll off on the shoulder and toe. They include:
    1. ***Very High Contrast***.
    1. ***High Contrast***.
    1. ***Medium High Contrast***.
    1. ***Base Contrast***. Similar to the sRGB contrast range, with a smoother toe.
    1. ***Medium Low Contrast***.
    1. ***Low Contrast***.
    1. ***Very Low Contrast***.
 1. ***False Colour***. This Look is an extremely useful tool for evaluating your image in terms of the dynamic range and latitude. It is a colour coded "heat map" of your image values, according to the following codes:

    | Value | Colour | Scene Referred Value |
    | ---- | ---- | ---- |
    | Low Clip | Black | Scene Referred Linear value below 0.0001762728758. |
    | -7.5 EV | Purple | Scene Referred Linear value 0.00099436891. |
    | -5.0 EV | Blue | Scene Referred Linear value 0.005625. |
    | -2.5 EV | Cyan | Scene Linear value 0.005625. |
    | -0.01 EV | Green-Cyan | Scene Referred Linear value ~0.178. |
    | 0 EV| Grey | Scene Referred Linear value 0.18009142. |
    | +0.01 EV | Green-Yellow | Scene Referred Linear value ~0.181. |
    | +2.17 EV | Yellow | Scene Referred Linear value 0.80779930598. |
    | +4.33 EV | Red | Scene Referred Linear value 3.62857262286. |
    | High Clip | White | Scene Referred Linear value above 16.29174024. |

# Colorimetric Information

## RGB Primaries

Filmic was designed to be easily integrated into existing pipelines, and as such, uses the primaries and achromatic white colour outlined in ITU-R BT.709 specification. As a result, the reference space lights match the sRGB specification and are as follows:

| Primary | x | y |
| -- | -- | -- |
| Red | 0.64 | 0.33 |
| Green | 0.30 | 0.60 |
| Blue | 0.15 | 0.06 |
| Achromatic | 0.3127 | 0.3290 |

## RGB to XYZ transform

Following with the above, the transformation for primaries to the CIE 1931 2 degree standard observer XYZ model, is:

| Primary | Red | Green | Blue |
| -- | -- | -- | -- |
| X | 0.4123910 | 0.3575840 | 0.1804810 |
| Y | 0.2126390 | 0.7151690 | 0.0721923 |
| Z | 0.0193308 | 0.1191950 | 0.9505320 |

## Transfer Function of Base Log Encoding

The canonized ground-truth encoding for Filmic is the *Filmic Base Log Encoding*. The Base Log Encoding is a pure normalized log 2 encoding in two parts, to accomodate the gamut mapping for wide primary ratio intensity gamut mapping. The first portion that is routed through the 3D LUT for the gamut compression at value range `-12.473931188, 12.526068812`. The final *Base Log Encoding* covers the log 2 range of `-12.473931188, 4.026068812`.

## Transfer Function of Aesthetic Outputs

The contrasts are designed for an idealized sRGB display, with the aforementionned primaries, and an Electro-Optical Transfer Function consisting of a pure 2.2 power function. BT.1886 does not integrate surround compensation, and is a 1:1 of the 2.2 assumptions.

# Issues

Please post any and all issues to the issue tracker at GitHub.
