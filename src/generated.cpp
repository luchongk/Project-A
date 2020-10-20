#include "game_state.h"

void GameState::initReflection(reflection::Type *CLASS) {
	CLASS->addField("reloadablesAlloc", &GameState::reloadablesAlloc)
		->addField("shaderManager", &GameState::shaderManager)
		->addField("VAO", &GameState::VAO)
		->addField("VBO", &GameState::VBO)
		->addField("texture", &GameState::texture)
		->addField("cameraPos", &GameState::cameraPos)
		->addField("cameraForward", &GameState::cameraForward)
		->addField("cameraYaw", &GameState::cameraYaw)
		->addField("cameraPitch", &GameState::cameraPitch)
		->addField("cubesRotationDir", &GameState::cubesRotationDir)
		->addField("cubesRotation", &GameState::cubesRotation)
		->addField("paused", &GameState::paused);
}

#include "linear_allocator.h"

void LinearAllocator::initReflection(reflection::Type *CLASS) {
	CLASS->addField("size", &LinearAllocator::size)
		->addField("used", &LinearAllocator::used)
		->addField("base", &LinearAllocator::base);
}

#include "shader_manager.h"

void ShaderManager::initReflection(reflection::Type *CLASS) {
	CLASS->addField("allocator", &ShaderManager::allocator)
		->addField("shaderMap", &ShaderManager::shaderMap);
}

#include "string_map.h"

template<typename T>
void StringMap<T>::Entry::initReflection(reflection::Type *CLASS) {
	CLASS->addField("key", &StringMap<T>::Entry::key)
		->addField("value", &StringMap<T>::Entry::value)
		->addField("next", &StringMap<T>::Entry::next);
}

template<typename T>
void StringMap<T>::initReflection(reflection::Type *CLASS) {
	CLASS->addField("buckets", &StringMap<T>::buckets)
		->addField("allocator", &StringMap<T>::allocator);
}

#include "vector.h"

void Vector3::initReflection(reflection::Type *CLASS) {
	CLASS->addField("x", &Vector3::x)
		->addField("y", &Vector3::y)
		->addField("z", &Vector3::z);
}

