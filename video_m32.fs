/*
Nuance pixel shader for 32-bit pixel modes:

This pixel shader displays one 32-bit YCrCbA format texture.  The texture 
coordinates should correspond to the main video channel.

The CCIR-601 definition of YCrCb is used.  This means that the valid integer range
of Y is [16,235] and the valid integer range of Cr and Cb is [16,240].

Note: Post-bias is not needed in this shader because the chromiance values
are being biased to [-0.5,0.5] unlike when using the imaging subset where
all intermediate values are required to stay within the range [0.0,1.0]
*/

uniform sampler2D mainChannelSampler;

const vec4 preBiasVector4 = vec4(16.0/255.0,16.0/255.0,16.0/255.0,0.0);
const vec4 chromianceBiasVector4 = vec4(0.0,0.5,0.5,1.0);
const vec4 expansionVector4 = vec4(255.0/219.0,255.0/224.0,255.0/224.0,1.0);
const mat4 ycrcb2rgbMatrix4 = mat4(1.000,1.402,0.000,0.000,1.000,-0.700,-0.340,0.000,1.000,0.000,1.772,0.000,0.000,0.000,0.000,1.000);

void main(void)
{
  vec4 mainColor = texture2D(mainChannelSampler, gl_TexCoord[0].st);

  mainColor.rgba = mainColor.bgra;
  mainColor = mainColor - preBiasVector4;
  mainColor = clamp((mainColor * expansionVector4), 0.0, 1.0);
  mainColor = mainColor - chromianceBiasVector4;
  gl_FragColor = clamp((mainColor * ycrcb2rgbMatrix4), 0.0, 1.0);
}
