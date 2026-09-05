#include "Game/FE/tlSlide.h"
#include "Game/FE/tlComponentInstance.h"
#include "Game/FE/feImage.h"

#include "NL/nlDLRing.h"

/**
 * Offset/Address/Size: 0x0 | 0x8020FBE0 | size: 0xD8
 */
void TLSlide::Update(float time)
{
    FEAnimation* anim = nlDLRingGetStart<FEAnimation>(m_animations);
    for (;;)
    {
        if (anim == NULL)
        {
            break;
        }
        anim->Update(m_time);
        if (nlDLRingIsEnd<FEAnimation>(m_animations, anim))
        {
            break;
        }
        anim = anim->m_next;
    }

    TLInstance* instance = nlDLRingGetStart<TLInstance>(m_instances);
    for (;;)
    {
        if (instance == NULL)
        {
            break;
        }
        if (instance->GetType() == TLAT_COMPONENT)
        {
            ((TLComponentInstance*)instance)->Update(time);
        }
        UpdateAsset(instance, time);
        if (nlDLRingIsEnd<TLInstance>(m_instances, instance))
        {
            break;
        }
        instance = instance->m_next;
    }
}

/**
 * Offset/Address/Size: 0xD8 | 0x8020FCB8 | size: 0x290
 */
void TLSlide::UpdateAsset(TLInstance* instance, float time)
{
    // Recursive walk over the instance tree. MWCC auto-inlines this recursion
    // seven levels deep, which is what the retail code shows.
    if (instance->pChildren == NULL)
    {
        return;
    }

    TLInstance* child = nlDLRingGetStart<TLInstance>(instance->pChildren);
    for (;;)
    {
        if (child->GetType() == TLAT_COMPONENT)
        {
            ((TLComponentInstance*)child)->Update(time);
        }
        UpdateAsset(child, time);
        if (nlDLRingIsEnd<TLInstance>(instance->pChildren, child))
        {
            break;
        }
        child = child->m_next;
    }
}
