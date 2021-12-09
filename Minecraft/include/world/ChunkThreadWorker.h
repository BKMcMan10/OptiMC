#ifndef MINECRAFT_CHUNK_THREAD_WORKER_H
#define MINECRAFT_CHUNK_THREAD_WORKER_H
#include "core.h"
// TODO: Remove this by getting rid of Pool type
#include "world/ChunkManager.h"

namespace Minecraft
{
	enum class CommandType : uint8
	{
		SaveBlockData = 0,
		ClientLoadChunk,
		GenerateTerrain,
		GenerateDecorations,
		CalculateLighting,
		RecalculateLighting,
		TesselateVertices
	};

	struct FillChunkCommand
	{
		// Must be at least ChunkWidth * ChunkDepth * ChunkHeight blocks available
		Chunk* chunk;
		Pool<SubChunk, World::ChunkCapacity * 16>* subChunks;
		glm::ivec2 playerPosChunkCoords;
		CommandType type;
		glm::vec3 blockThatUpdated;
		bool removedLightSource;
		void* clientChunkData;
	};

	struct CompareFillChunkCommand
	{
		// Returning true means lesser priority
		bool operator()(const FillChunkCommand& a, const FillChunkCommand& b) const;
	};

	class ChunkThreadWorker
	{
	public:
		ChunkThreadWorker();

		void free();

		void threadWorker();
		void queueCommand(FillChunkCommand& command);

		void beginWork(bool notifyAll = true);
		void setPlayerPosChunkCoords(const glm::ivec2& playerPosChunkCoords);

	private:
		std::priority_queue<FillChunkCommand, std::vector<FillChunkCommand>, CompareFillChunkCommand> commands;
		std::thread workerThread;
		std::atomic<glm::ivec2> playerPosChunkCoords;
		std::condition_variable cv;
		std::mutex mtx;
		std::mutex queueMtx;
		bool doWork;
		std::atomic<bool> waitingOnCommand = false;

		std::array<SimplexNoise, 5> noiseGenerators;
	};
}

#endif 