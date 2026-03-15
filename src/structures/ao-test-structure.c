#include "structure.h"
#include "../block-model.h"

void structure_ao_test(struct World *world, struct BlockPos origin) {
	assert(world != NULL);

	struct BlockPos pos = {origin.x, origin.y, origin.z};

	// AO Case 0A
	pos.x += 4;
	pos.y = origin.y;
	pos.z = origin.z;

	world_set_block(world, pos.x, pos.y, pos.z, STONE_BLOCK);		  // Base
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z, STONE_BLOCK); // Side 1
	world_set_block(world, pos.x, pos.y + 1, pos.z + 1, STONE_BLOCK); // Side 2

	// AO Case 0B
	pos.z += 4;

	world_set_block(world, pos.x, pos.y, pos.z, STONE_BLOCK);			  // Base
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z, STONE_BLOCK);	  // Side 1
	world_set_block(world, pos.x, pos.y + 1, pos.z + 1, STONE_BLOCK);	  // Side 2
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z + 1, STONE_BLOCK); // Corner

	// AO Case 1A
	pos.x += 4;
	pos.y = origin.y;
	pos.z = origin.z;

	world_set_block(world, pos.x, pos.y, pos.z, STONE_BLOCK);			  // Base
	world_set_block(world, pos.x, pos.y + 1, pos.z + 1, STONE_BLOCK);	  // Side 1
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z + 1, STONE_BLOCK); // Corner

	// AO Case 1B
	pos.z += 4;

	world_set_block(world, pos.x + 1, pos.y, pos.z, STONE_BLOCK);		  // Base
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z + 1, STONE_BLOCK); // Side 1
	world_set_block(world, pos.x, pos.y + 1, pos.z + 1, STONE_BLOCK);	  // Corner

	// AO Case 2A
	pos.x += 4;
	pos.y = origin.y;
	pos.z = origin.z;

	world_set_block(world, pos.x, pos.y, pos.z, STONE_BLOCK);		  // Base
	world_set_block(world, pos.x, pos.y + 1, pos.z + 1, STONE_BLOCK); // Side 1

	// AO Case 2B
	pos.z += 4;

	world_set_block(world, pos.x, pos.y, pos.z, STONE_BLOCK);			  // Base
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z + 1, STONE_BLOCK); // Side 1

	// AO Case 3
	pos.x += 4;
	pos.y = origin.y;
	pos.z = origin.z;

	world_set_block(world, pos.x, pos.y, pos.z, STONE_BLOCK); // Base

	//
	// Rotated 90 degrees clockwise
	//

	// AO Case 0A
	pos.x += 4;
	pos.y = origin.y;
	pos.z = origin.z;

	world_set_block(world, pos.x + 1, pos.y, pos.z, DIRT_BLOCK);		 // Base
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z + 1, DIRT_BLOCK); // Side 1
	world_set_block(world, pos.x, pos.y + 1, pos.z, DIRT_BLOCK);		 // Side 2

	// AO Case 0B
	pos.z += 4;

	world_set_block(world, pos.x + 1, pos.y, pos.z, DIRT_BLOCK);		 // Base
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z + 1, DIRT_BLOCK); // Side 1
	world_set_block(world, pos.x, pos.y + 1, pos.z, DIRT_BLOCK);		 // Side 2
	world_set_block(world, pos.x, pos.y + 1, pos.z + 1, DIRT_BLOCK);	 // Corner

	// AO Case 1A
	pos.x += 4;
	pos.y = origin.y;
	pos.z = origin.z;

	world_set_block(world, pos.x + 1, pos.y, pos.z + 1, DIRT_BLOCK); // Base
	world_set_block(world, pos.x, pos.y + 1, pos.z + 1, DIRT_BLOCK); // aSide 1
	world_set_block(world, pos.x, pos.y + 1, pos.z, DIRT_BLOCK);	 // Corner

	// AO Case 1B
	pos.z += 4;

	world_set_block(world, pos.x + 1, pos.y, pos.z, DIRT_BLOCK);	 // Base
	world_set_block(world, pos.x, pos.y + 1, pos.z, DIRT_BLOCK);	 // Side 1
	world_set_block(world, pos.x, pos.y + 1, pos.z + 1, DIRT_BLOCK); // Corner

	// AO Case 2A
	pos.x += 4;
	pos.y = origin.y;
	pos.z = origin.z;

	world_set_block(world, pos.x + 1, pos.y, pos.z, DIRT_BLOCK); // Base
	world_set_block(world, pos.x, pos.y + 1, pos.z, DIRT_BLOCK); // Side 1

	// AO Case 2B
	pos.z += 4;

	world_set_block(world, pos.x + 1, pos.y, pos.z, DIRT_BLOCK);	 // Base
	world_set_block(world, pos.x, pos.y + 1, pos.z + 1, DIRT_BLOCK); // Side 1

	// AO Case 3
	pos.x += 4;
	pos.y = origin.y;
	pos.z = origin.z;

	world_set_block(world, pos.x, pos.y, pos.z, DIRT_BLOCK); // Base

	//
	// Rotated 90 degrees clockwise
	//

	// AO Case 0A
	pos.x += 4;
	pos.y = origin.y;
	pos.z = origin.z;

	world_set_block(world, pos.x + 1, pos.y, pos.z + 1, WATER_BLOCK); // Base
	world_set_block(world, pos.x, pos.y + 1, pos.z + 1, WATER_BLOCK); // Side 1
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z, WATER_BLOCK); // Side 2

	// AO Case 0B
	pos.z += 4;

	world_set_block(world, pos.x + 1, pos.y, pos.z + 1, WATER_BLOCK); // Base
	world_set_block(world, pos.x, pos.y + 1, pos.z + 1, WATER_BLOCK); // Side 1
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z, WATER_BLOCK); // Side 2
	world_set_block(world, pos.x, pos.y + 1, pos.z, WATER_BLOCK);	  // Corner

	// AO Case 1A
	pos.x += 4;
	pos.y = origin.y;
	pos.z = origin.z;

	world_set_block(world, pos.x, pos.y, pos.z + 1, WATER_BLOCK);	  // Base
	world_set_block(world, pos.x, pos.y + 1, pos.z, WATER_BLOCK);	  // Side 1
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z, WATER_BLOCK); // Corner

	// AO Case 1B
	pos.z += 4;

	world_set_block(world, pos.x + 1, pos.y, pos.z + 1, WATER_BLOCK); // Base
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z, WATER_BLOCK); // Side 1
	world_set_block(world, pos.x, pos.y + 1, pos.z, WATER_BLOCK);	  // Corner

	// AO Case 2A
	pos.x += 4;
	pos.y = origin.y;
	pos.z = origin.z;

	world_set_block(world, pos.x, pos.y, pos.z + 1, WATER_BLOCK); // Base
	world_set_block(world, pos.x, pos.y + 1, pos.z, WATER_BLOCK); // Side 1

	// AO Case 2B
	pos.z += 4;

	world_set_block(world, pos.x + 1, pos.y, pos.z + 1, WATER_BLOCK); // Base
	world_set_block(world, pos.x, pos.y + 1, pos.z, WATER_BLOCK);	  // Side 1

	// AO Case 3
	pos.x += 4;
	pos.y = origin.y;
	pos.z = origin.z;

	world_set_block(world, pos.x, pos.y, pos.z, WATER_BLOCK); // Base

	//
	// Rotated 90 degrees clockwise
	//

	// AO Case 0A
	pos.x += 4;
	pos.y = origin.y;
	pos.z = origin.z;

	world_set_block(world, pos.x, pos.y, pos.z + 1, GRASS_BLOCK);		  // Base
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z + 1, GRASS_BLOCK); // Side 1
	world_set_block(world, pos.x, pos.y + 1, pos.z, GRASS_BLOCK);		  // Side 2

	// AO Case 0B
	pos.z += 4;

	world_set_block(world, pos.x, pos.y, pos.z + 1, GRASS_BLOCK);		  // Base
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z + 1, GRASS_BLOCK); // Side 1
	world_set_block(world, pos.x, pos.y + 1, pos.z, GRASS_BLOCK);		  // Side 2
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z, GRASS_BLOCK);	  // Corner

	// AO Case 1A
	pos.x += 4;
	pos.y = origin.y;
	pos.z = origin.z;

	world_set_block(world, pos.x, pos.y, pos.z, GRASS_BLOCK);			  // Base
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z + 1, GRASS_BLOCK); // Side 1
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z, GRASS_BLOCK);	  // Corner

	// AO Case 1B
	pos.z += 4;

	world_set_block(world, pos.x, pos.y, pos.z + 1, GRASS_BLOCK);		  // Base
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z + 1, GRASS_BLOCK); // Side 1
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z, GRASS_BLOCK);	  // Corner

	// AO Case 2A
	pos.x += 4;
	pos.y = origin.y;
	pos.z = origin.z;

	world_set_block(world, pos.x, pos.y, pos.z, GRASS_BLOCK);		  // Base
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z, GRASS_BLOCK); // Side 1

	// AO Case 2B
	pos.z += 4;

	world_set_block(world, pos.x, pos.y, pos.z, GRASS_BLOCK);			  // Base
	world_set_block(world, pos.x + 1, pos.y + 1, pos.z + 1, GRASS_BLOCK); // Side 1

	// AO Case 3
	pos.x += 4;
	pos.y = origin.y;
	pos.z = origin.z;

	world_set_block(world, pos.x, pos.y, pos.z, GRASS_BLOCK); // Base
}
