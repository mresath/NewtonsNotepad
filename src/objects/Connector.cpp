#include "Connector.hpp"

/* CONNECTOR */

Connector::Connector(Object *objA, Vec2 ancA, Object *objB, Vec2 ancB)
    : objectA(objA), anchorA(ancA), objectB(objB), anchorB(ancB) {}

Connector::~Connector()
{
    removeForces();
}

ForceSource Connector::getForceSource(bool isObjectA) const
{
    return ForceSource(getIDString());
}

void Connector::connectForces() const
{
    if (objectA != nullptr)
        objectA->applyForce(getForceSource(true));
    if (objectB != nullptr)
        objectB->applyForce(getForceSource(false));
}

void Connector::removeForces() const
{
    if (objectA != nullptr)
        objectA->deleteForce(this->getIDString());
    if (objectB != nullptr)
        objectB->deleteForce(this->getIDString());
}

void Connector::draw(sf::RenderWindow *window) const
{
    Vec2 posA = objectA != nullptr ? objectA->body->position + anchorA : anchorA;
    Vec2 posB = objectB != nullptr ? objectB->body->position + anchorB : anchorB;

    Vec2 pixelsA = *metersToPixels(&posA);
    Vec2 pixelsB = *metersToPixels(&posB);

    sf::Vertex v1{{pixelsA.x, pixelsA.y}, CONNECTOR_COLOR};
    sf::Vertex v2{{pixelsB.x, pixelsB.y}, CONNECTOR_COLOR};

    sf::Vertex line[] = {v1, v2};

    window->draw(line, 2, sf::PrimitiveType::Lines);
}

sf::RectangleShape Connector::getBoundingBox() const
{
    Vec2 posA = objectA != nullptr ? objectA->body->position + anchorA : anchorA;
    Vec2 posB = objectB != nullptr ? objectB->body->position + anchorB : anchorB;

    Vec2 pixelsA = *metersToPixels(&posA);
    Vec2 pixelsB = *metersToPixels(&posB);

    float theta = (pixelsB - pixelsA).angle();
    float length = (pixelsB - pixelsA).length();

    sf::RectangleShape box(sf::Vector2f(length, CONNECTOR_THICKNESS));
    box.setOrigin(sf::Vector2f(0, CONNECTOR_THICKNESS / 2));
    box.setPosition(sf::Vector2f(pixelsA.x, pixelsA.y));
    box.setRotation(sf::radians(theta));

    return box;
}

void Connector::setID(int newID)
{
    id = newID;
}

int Connector::getID() const
{
    return id;
}

std::string Connector::getIDString() const
{
    return "connector_" + std::to_string(id);
}

/* SPRING */

Spring::Spring(Object *objA, Vec2 ancA, Object *objB, Vec2 ancB, float stiff, float damp, float restLen)
    : Connector(objA, ancA, objB, ancB), stiffness(stiff), damping(damp), restingLength(restLen) {}

ForceSource Spring::getForceSource(bool isObjectA) const
{
    const Vec2 *ancAptr = &anchorA;
    const Vec2 *ancBptr = &anchorB;

    return ForceSource(getIDString(), [this, ancAptr, ancBptr, isObjectA](const Body &state) -> Force
                       {
        Vec2 anchor = isObjectA ? *ancAptr : *ancBptr;
        Vec2 origin = state.position + anchor;

        Object *otherObject = isObjectA ? objectB : objectA;
        Vec2 otherAnchor = isObjectA ? anchorB : anchorA;
        Vec2 otherPosition = otherObject != nullptr ? otherObject->body->position + otherAnchor : otherAnchor;

        Vec2 dir = origin - otherPosition;
        float currentLength = dir.length();
        if (currentLength == 0.0f)
            return Force(anchor, Vec2(0.0f, 0.0f));
        
        Vec2 unitDir = dir / currentLength;
        float lengthDiff = currentLength - restingLength;
        Vec2 relativeVelocity = otherObject != nullptr ? state.velocity - otherObject->body->velocity : state.velocity;
        float velAlongDir = dot(relativeVelocity, unitDir);
        Vec2 springForce = unitDir * (-stiffness * lengthDiff - damping * velAlongDir);
        return Force(anchor, springForce); });
}

void Spring::draw(sf::RenderWindow *window) const
{
    Vec2 posA = objectA != nullptr ? objectA->body->position + anchorA : anchorA;
    Vec2 posB = objectB != nullptr ? objectB->body->position + anchorB : anchorB;

    Vec2 pixelsA = *metersToPixels(&posA);
    Vec2 pixelsB = *metersToPixels(&posB);

    // Calculate spring direction and length
    Vec2 direction = pixelsB - pixelsA;
    float length = direction.length();

    if (length < 1.0f)
        return; // Don't draw if too short

    Vec2 unitDir = direction / length;
    Vec2 perpendicular(-unitDir.y, unitDir.x);

    // Spring visualization parameters
    const int coilCount = 12;        // Number of coils
    const float amplitude = 5.0f;    // Width of the spring coils
    const float endCapLength = 8.0f; // Length of straight segments at ends

    // Create vertices for the spring shape
    std::vector<sf::Vertex> vertices;

    // Start cap (straight line)
    sf::Vertex vStart1{{pixelsA.x, pixelsA.y}, CONNECTOR_COLOR};
    vertices.push_back(vStart1);
    Vec2 startCoil = pixelsA + unitDir * endCapLength;
    sf::Vertex vStart2{{startCoil.x, startCoil.y}, CONNECTOR_COLOR};
    vertices.push_back(vStart2);

    // Calculate the coil section length
    float coilSectionLength = length - 2.0f * endCapLength;

    if (coilSectionLength > 0.0f)
    {
        // Draw the coils
        for (int i = 0; i <= coilCount; i++)
        {
            float t = static_cast<float>(i) / static_cast<float>(coilCount);
            float offset = (i % 2 == 0) ? amplitude : -amplitude;

            Vec2 point = startCoil + unitDir * (t * coilSectionLength) + perpendicular * offset;
            sf::Vertex v{{point.x, point.y}, CONNECTOR_COLOR};
            vertices.push_back(v);
        }
    }

    // End cap (straight line)
    Vec2 endCoil = pixelsB - unitDir * endCapLength;
    sf::Vertex vEnd1{{endCoil.x, endCoil.y}, CONNECTOR_COLOR};
    sf::Vertex vEnd2{{pixelsB.x, pixelsB.y}, CONNECTOR_COLOR};
    vertices.push_back(vEnd1);
    vertices.push_back(vEnd2);

    // Draw the spring
    window->draw(vertices.data(), vertices.size(), sf::PrimitiveType::LineStrip);
}

/* ROPE */
Rope::Rope(Object *objA, Vec2 ancA, Object *objB, Vec2 ancB, float length, float segments, float segMass)
    : Connector(objA, ancA, objB, ancB), totalLength(length), segmentCount(segments), segmentMass(segMass) {}

ForceSource Rope::getForceSource(bool isObjectA) const
{
    return ForceSource(getIDString());
}

void Rope::draw(sf::RenderWindow *window) const
{
    Connector::draw(window);
}