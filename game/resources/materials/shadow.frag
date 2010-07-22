
#version 120

varying float y; // Interpolated y coordinate of shadow-casting vertex

void main(void)
{
   if (y < 0)
     discard; // Prevents submerged creatures from casting a shadow
   gl_FragColor = vec4(0, 0, 0, 0.5);
}
