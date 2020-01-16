/*
Nuance pixel shader for 32-bit pixel modes:

This pixel shader blends together two 16-bit YCrCb format textures.  The alpha
component for the overlay channel should be specified as a measure of transparency
and not as a measure of opacity as is standard in OpenGL.  The first set of texture 
coordinates should correspond to the main video channel and the second set of texture
coordinates should correspond to the overlay video channel.  Invalid YCrCbA pixels of
(0.0, 0.0, 0.0, X.X) are considered to be transparent for the overlay channel only.

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
uniform float constant_alpha;

const vec3 zeroVector3 = vec3(0.0,0.0,0.0);
const vec4 zeroVector4 = vec4(0.0,0.0,0.0,0.0);
const vec4 oneVector4 = vec4(1.0,1.0,1.0,1.0);
const vec4 preBiasVector4 = vec4(16.0/255.0,16.0/255.0,16.0/255.0,0.0);
const vec4 chromianceBiasVector4 = vec4(0.0,0.5,0.5,1.0);
const vec4 expansionVector4 = vec4(255.0/219.0,255.0/224.0,255.0/224.0,1.0);
const mat4 ycrcb2rgbMatrix4 = mat4(1.000,1.402,0.000,0.000,1.000,-0.700,-0.340,0.000,1.000,0.000,1.772,0.000,0.000,0.000,0.000,1.000);


void main(void)
{
  vec4 temp;
  vec4 mainColor = texture2D(mainChannelSampler, gl_TexCoord[0].st);
  vec4 overlayColor = texture2D(overlayChannelSampler, gl_TexCoord[1].st);

  //Convert from CrYCbA (5:5:5:1) to YCrCbA (6:5:5:0)
  mainColor.rgb = mainColor.bgr;
  //Fix Y: float equivalent of Y = (temp << 1) | a
  mainColor.r = (temp * 2.0) + mainColor.a;
  mainColor.a = 0.0;

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
    //Convert from CrYCbA (5:5:5:1) to YCrCbA (6:5:5:0)
    temp = overlayColor.b;
    //Fix Cb
    overlayColor.b = overlayColor.g;
    //Fix Cr
    overlayColor.g = overlayColor.r;
    //Fix Y: float equivalent of Y = (temp << 1) | a
    overlayColor.r = (temp * 2.0) + overlayColor.a;
    overlayColor.a = constant_alpha;

    overlayColor = overlayColor - preBiasVector4;
    overlayColor = clamp((overlayColor * expansionVector4), 0.0, 1.0);
    overlayColor = overlayColor - chromianceBiasVector4;
    overlayColor = clamp((overlayColor * ycrcb2rgbMatrix4), 0.0, 1.0);

    //(main*(1 - constant_alpha) + overlay*(constant_alpha)
    gl_FragColor = mix(mainColor, overlayColor, constant_alpha);
  }
}
