#pragma once

#ifndef PARTING_IID_PPV_ARGS
#define PARTING_IID_PPV_ARGS(ppType) __uuidof(**(ppType)), IID_PPV_ARGS_Helper(ppType)
#endif // !IID_PPV_ARGS
