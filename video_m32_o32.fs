/*
Nuance pixel shader for 16/32-bit pixel modes:

This pixel shader blends together two 16/32-bit YCrCbA format textures.  The alpha
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
uniform sampler2D LUTSampler;

uniform float structOverlayChannelAlpha;
uniform float mainIs16bit;

uniform float resy;

const vec3 preBias = vec3(16.0/255.0,16.0/255.0,16.0/255.0);
const vec2 chromianceBias = vec2(0.5,0.5);
const vec3 expansion = vec3(255.0/219.0,255.0/224.0,255.0/224.0);
const mat3 ycrcb2rgb = mat3(1.000,1.402,0.000,1.000,-0.714136,-0.34413,1.000,0.000,1.772);

vec4 texture2D_main(vec2 uv_org)
{
  vec4 bilerp[4];
  for(int i = 0; i < 4; ++i)
  {
    vec2 uv = uv_org;
    if(i == 1 || i == 3)
      uv.x += 1.;
    if(i > 1)
      uv.y += 1.;
    uv *= vec2(0.0013888889,1./resy); // 1./720

    vec4 mainColor;
    if(mainIs16bit != 0.)
    {
      vec2 mainColor88 = texture2D(mainChannelSampler, uv).yx;
      mainColor = vec4(texture2D(LUTSampler, mainColor88).xyz,1.0);
    }
    else
      mainColor = texture2D(mainChannelSampler, uv).bgra;

    mainColor.rgb -= preBias;
    mainColor.rgb = clamp(mainColor.rgb * expansion, 0.0, 1.0);
    mainColor.gb -= chromianceBias;
    mainColor.rgb = clamp(mainColor.rgb * ycrcb2rgb, 0.0, 1.0);

    bilerp[i] = mainColor;
  }

  vec2 bilerpw = fract(uv_org);
  // smoothstep:
  bilerpw = bilerpw*bilerpw*(3.-2.*bilerpw); //bilerpw = bilerpw*bilerpw*bilerpw*(bilerpw*(bilerpw*6.0-15.0)+10.0);
  return   bilerp[0]*((1.-bilerpw.x)*(1.-bilerpw.y))
         + bilerp[1]*(    bilerpw.x *(1.-bilerpw.y))
         + bilerp[2]*((1.-bilerpw.x)*    bilerpw.y)
         + bilerp[3]*(    bilerpw.x *    bilerpw.y);
}

vec4 texture2D_overlay(vec2 uv_org)
{
  vec4 bilerp[4];
  for(int i = 0; i < 4; ++i)
  {
    vec2 uv = uv_org;
    if(i == 1 || i == 3)
      uv.x += 1.;
    if(i > 1)
      uv.y += 1.;
    uv *= vec2(0.0013888889,1./resy); // 1./720

    vec4 overlayColor;
    if(structOverlayChannelAlpha >= 0.)
    {
      vec2 overlayColor88 = texture2D(overlayChannelSampler, uv).yx;
      overlayColor = vec4(texture2D(LUTSampler, overlayColor88).xyz,structOverlayChannelAlpha);
    }
    else
      overlayColor = texture2D(overlayChannelSampler, uv).bgra;

    if(overlayColor.rgb != vec3(0.0,0.0,0.0)) //!! necessary?! seems not
    {
      overlayColor.rgb -= preBias;
      overlayColor.rgb = clamp(overlayColor.rgb * expansion, 0.0, 1.0);
      overlayColor.gb -= chromianceBias;
      overlayColor.rgb = clamp(overlayColor.rgb * ycrcb2rgb, 0.0, 1.0);
    }

    bilerp[i] = overlayColor;
  }

  vec2 bilerpw = fract(uv_org);
  // smoothstep:
  bilerpw = bilerpw*bilerpw*(3.-2.*bilerpw); //bilerpw = bilerpw*bilerpw*bilerpw*(bilerpw*(bilerpw*6.0-15.0)+10.0);
  return   bilerp[0]*((1.-bilerpw.x)*(1.-bilerpw.y))
         + bilerp[1]*(    bilerpw.x *(1.-bilerpw.y))
         + bilerp[2]*((1.-bilerpw.x)*    bilerpw.y)
         + bilerp[3]*(    bilerpw.x *    bilerpw.y);
}

vec4 texture2D_composite(vec2 mainuv, vec2 overlayuv)
{
  vec4 mainColor = texture2D_main(mainuv);
  vec4 overlayColor = texture2D_overlay(overlayuv);

  if(overlayColor.rgb == vec3(0.0,0.0,0.0))
    return mainColor;
  else
  {
    //On the Nuon, alpha represents transparency with 0x00 being fully opaque.
    return vec4(mix(mainColor.rgb, overlayColor.rgb, 1.0 - overlayColor.a), 1.0);
  }
}

void main()
{
  vec2 mainuv    = gl_TexCoord[0].st*vec2(720.,resy)-0.499; //!! 0.499 because of precision problems if src width/height is rather small (Space Invaders XL)
  vec2 overlayuv = gl_TexCoord[1].st*vec2(720.,resy)-0.499; //!! dto.

  gl_FragColor = texture2D_composite(mainuv, overlayuv);
}
