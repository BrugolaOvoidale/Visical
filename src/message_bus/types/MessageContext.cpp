#include "MessageContext.hpp"
#include <stdexcept>


MessageContext::MessageContext(const std::unordered_map<std::string, MessageContext::Attribute>& attributes)
	: attributes_(attributes)
{
}

MessageContext::MessageContext(std::initializer_list<std::pair<const std::string, MessageContext::Attribute>> init)
    : attributes_(init)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////

MessageContext& MessageContext::setAttributes(const std::unordered_map<std::string, MessageContext::Attribute>& attributes)
{
    attributes_ = attributes;

    return *this;
}

MessageContext& MessageContext::setAttribute(const std::string& key, bool value)
{
    attributes_[key] = value;

    return *this;
}

MessageContext& MessageContext::setAttribute(const std::string& key, const std::string& value)
{
    attributes_[key] = value;

    return *this;
}

std::optional<MessageContext::Attribute> MessageContext::getVariantAttribute(const std::string& key) const
{
    auto it = attributes_.find(key);
    if (it == attributes_.end())
        return std::nullopt;

    return it->second;
}

const std::unordered_map<std::string, MessageContext::Attribute>& MessageContext::getAttributes() const
{
    return attributes_;
}
