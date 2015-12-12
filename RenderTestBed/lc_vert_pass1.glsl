#version 330

layout(location = 0)in vec3 vertex;
layout(location = 1)in vec3 normal;
layout(location = 2)in vec4 color;
layout(location = 3)in vec3 uv;

out vec4 outColor;
out vec3 outUV;
out vec3 origVertex;// L

out vec3 varyingNormal;

// UNIFORM
uniform int mode;

// MODE 1--> color
// MODE 2--> texture
// MODE 3--> terrain
// MODE 4--> water
// MODE 5--> model obj: one color
// MODE 6--> model obj: texture

//0x0100 --> adapt terrain
//0x0200 --> lighting

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

// terrain
uniform vec4 terrain_size;
uniform sampler2D terrain_tex;

// model
uniform vec4 justOneColor;
uniform mat4 modelTransf;

void main(){
	
	outColor=color;
	outUV=uv;
	origVertex=vertex;

	// 2. ADAPT TO TERRAIN
	if(((mode&0xFF)==0x03)||((mode&0x0100)==0x0100)){// terrain or adapt to terrain
		vec2 terrainTexCoord=vec2(
			(origVertex.x-terrain_size.x)/terrain_size.z,
			(origVertex.y-terrain_size.y)/terrain_size.w
			);
		//float height=255.0f*length(texture(terrain_tex,terrainTexCoord.rg));
		float height=255.0f*texture(terrain_tex,terrainTexCoord.rg).r;
		origVertex.z+=height;
		//if(height<15.0f)//water height
		//	origVertex.z=-100.0f;
	}

	// LIGHTING
	if((mode&0x0200)==0x0200){
		varyingNormal=normal;
		// TRANSFORM MODEL (it should change its normal too)
		if(((mode&0x0FF)==0x05)||((mode&0xFF)==0x06)){
			varyingNormal=normal.xzy;
			varyingNormal=(modelTransf*vec4(varyingNormal,0.0)).xyz;//note 0.0
		}
	}else{
		varyingNormal=vec3(0);//no lighting (check when lighting)
	}

	gl_Position = mvpMatrix * vec4(origVertex,1.0);

}