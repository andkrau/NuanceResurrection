/*
Nuance pixel shader for 32-bit pixel modes:

This pixel shader blends together two 32-bit YCrCbA format textures.  The alpha
component for each pixel is expected to be a measure of transparency and not
opacity as is usually the case in OpenGL.  The first set of texture coordinates
should correspond to the main video channel and the second set of texture 
coordinates should correspond to the overlay video channel.  Invalid YCrCbA
pixels of (0.0, 0.0, 0.0, X.X) are considered to be transparent for the overlay
channel only.

The CCIR-601 definition of YCrCb is used.  This means that the valid integer range
of Y is [16,235] and the valid integer range of Cr and Cb is [16,240].

After converting the texels to RGBA, the main channel texel is blended 
with the overlay channel texel using the the overlay texel alpha value to
determine the transparency factor for the overlay color.  That is, the final
color is [mainColor * (1 - overlayColor.a)] + (overlayColor * overlayColor.a)

Note: Post-bias is not needed in this shader because the chromiance values
are being biased to [-0.5,0.5] unlike when using the imaging subset where
all intermediate values are required to stay within the range [0.0,1.0]
*/

uniform sampler2D mainChannelSampler;
uniform sampler2D overlayChannelSampler;

const vec3 zeroVector3 = vec3(0.0,0.0,0.0);
const vec4 preBiasVector4 = vec4(16.0/255.0,16.0/255.0,16.0/255.0,0.0);
const vec4 chromianceBiasVector4 = vec4(0.0,0.5,0.5,0.0);
const vec4 expansionVector4 = vec4(255.0/219.0,255.0/224.0,255.0/224.0,1.0);
const mat4 ycrcb2rgbMatrix4 = mat4(1.000,1.402,0.000,0.000,1.000,-0.714136,-0.34413,0.000,1.000,0.000,1.772,0.000,0.000,0.000,0.000,1.000);

void main(void)
{
  vec4 mainColor = texture2D(mainChannelSampler, gl_TexCoord[0].st);
  vec4 overlayColor = texture2D(overlayChannelSampler, gl_TexCoord[1].st);
  
  mainColor.rgba = mainColor.bgra;
  overlayColor.rgba = overlayColor.bgra;

  mainColor = mainColor - preBiasVector4;
  mainColor = clamp((mainColor * expansionVector4), 0.0, 1.0);
  mainColor = mainColor - chromianceBiasVector4;
  mainColor = clamp((mainColor * ycrcb2rgbMatrix4), 0.0, 1.0);

  if(overlayColor.rgb == zeroVector3)
  {
    gl_FragColor = mainColor;
  }
  else
  {
    overlayColor = overlayColor - preBiasVector4;
    overlayColor = clamp((overlayColor * expansionVector4), 0.0, 1.0);
    overlayColor = overlayColor - chromianceBiasVector4;
    overlayColor = clamp((overlayColor * ycrcb2rgbMatrix4), 0.0, 1.0);
 
    //On the Nuon, alpha represents transparency with 0x00 being fully opaque.
    //color = overlayColor*(1 - overlay.a) + mainColor*(overlay.a)
    gl_FragColor = mix(mainColor, overlayColor, 1 - overlayColor.a);
  }
}
