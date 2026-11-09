target_sources(
	${CMAKE_PROJECT_NAME} PRIVATE

	Authenticator.cpp
	ChunkDataSerializer.cpp
	ForgeHandshake.cpp
	MojangAPI.cpp
	Packetizer.cpp
	Protocol_1_12.cpp
	ProtocolRecognizer.cpp
	RecipeMapper.cpp
	Palettes/Upgrade.cpp

	Authenticator.h
	ChunkDataSerializer.h
	ForgeHandshake.h
	MojangAPI.h
	Packetizer.h
	Protocol.h
	Protocol_1_12.h
	ProtocolRecognizer.h
	RecipeMapper.h
	Palettes/Upgrade.h
)