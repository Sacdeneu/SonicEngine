#include "scene.h"

Scene* Scene::mainScene = NULL;

Scene::Scene()
{
}

int Scene::GetObjectsCount()
{
	return gameObjects.size();
}

void Scene::AddParticle(Particle* p)
{
	gameObjects.push_back(p);
}

void Scene::Update(float dt)
{
	for (int i = 0; i < gameObjects.size(); i++)
	{
		gameObjects[i]->Update(dt);
	}
}

void Scene::RemoveParticle(Particle* p)
{
	gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), p), gameObjects.end());
	delete p;
}
