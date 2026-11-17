/*******************************************************************************************************************
Copyright (c) 2023 Cycling '74

The code that Max generates automatically and that end users are capable of
exporting and using, and any associated documentation files (the “Software”)
is a work of authorship for which Cycling '74 is the author and owner for
copyright purposes.

This Software is dual-licensed either under the terms of the Cycling '74
License for Max-Generated Code for Export, or alternatively under the terms
of the General Public License (GPL) Version 3. You may use the Software
according to either of these licenses as it is most appropriate for your
project on a case-by-case basis (proprietary or not).

A) Cycling '74 License for Max-Generated Code for Export

A license is hereby granted, free of charge, to any person obtaining a copy
of the Software (“Licensee”) to use, copy, modify, merge, publish, and
distribute copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The Software is licensed to Licensee for all uses that do not include the sale,
sublicensing, or commercial distribution of software that incorporates this
source code. This means that the Licensee is free to use this software for
educational, research, and prototyping purposes, to create musical or other
creative works with software that incorporates this source code, or any other
use that does not constitute selling software that makes use of this source
code. Commercial distribution also includes the packaging of free software with
other paid software, hardware, or software-provided commercial services.

For entities with UNDER $200k in annual revenue or funding, a license is hereby
granted, free of charge, for the sale, sublicensing, or commercial distribution
of software that incorporates this source code, for as long as the entity's
annual revenue remains below $200k annual revenue or funding.

For entities with OVER $200k in annual revenue or funding interested in the
sale, sublicensing, or commercial distribution of software that incorporates
this source code, please send inquiries to licensing@cycling74.com.

The above copyright notice and this license shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Please see
https://support.cycling74.com/hc/en-us/articles/10730637742483-RNBO-Export-Licensing-FAQ
for additional information

B) General Public License Version 3 (GPLv3)
Details of the GPLv3 license can be found at: https://www.gnu.org/licenses/gpl-3.0.html
*******************************************************************************************************************/

#ifdef RNBO_LIB_PREFIX
#define STR_IMPL(A) #A
#define STR(A) STR_IMPL(A)
#define RNBO_LIB_INCLUDE(X) STR(RNBO_LIB_PREFIX/X)
#else
#define RNBO_LIB_INCLUDE(X) #X
#endif // RNBO_LIB_PREFIX
#ifdef RNBO_INJECTPLATFORM
#define RNBO_USECUSTOMPLATFORM
#include RNBO_INJECTPLATFORM
#endif // RNBO_INJECTPLATFORM

#include RNBO_LIB_INCLUDE(RNBO_Common.h)
#include RNBO_LIB_INCLUDE(RNBO_AudioSignal.h)

namespace RNBO {


#define trunc(x) ((Int)(x))
#define autoref auto&

#if defined(__GNUC__) || defined(__clang__)
    #define RNBO_RESTRICT __restrict__
#elif defined(_MSC_VER)
    #define RNBO_RESTRICT __restrict
#endif

#define FIXEDSIZEARRAYINIT(...) { }

template <class ENGINE = INTERNALENGINE> class rnbomatic : public PatcherInterfaceImpl {

friend class EngineCore;
friend class Engine;
friend class MinimalEngine<>;
public:

rnbomatic()
: _internalEngine(this)
{
}

~rnbomatic()
{
    deallocateSignals();
}

Index getNumMidiInputPorts() const {
    return 0;
}

void processMidiEvent(MillisecondTime , int , ConstByteArray , Index ) {}

Index getNumMidiOutputPorts() const {
    return 0;
}

void process(
    const SampleValue * const* inputs,
    Index numInputs,
    SampleValue * const* outputs,
    Index numOutputs,
    Index n
) {
    RNBO_UNUSED(numInputs);
    RNBO_UNUSED(inputs);
    this->vs = n;
    this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr, true);
    SampleValue * out1 = (numOutputs >= 1 && outputs[0] ? outputs[0] : this->dummyBuffer);
    SampleValue * out2 = (numOutputs >= 2 && outputs[1] ? outputs[1] : this->dummyBuffer);
    this->noise_tilde_01_perform(this->signals[0], n);
    this->pink_tilde_01_perform(this->signals[1], n);
    this->dspexpr_04_perform(this->signals[1], this->dspexpr_04_in2, this->signals[2], n);

    this->dspexpr_03_perform(
        this->signals[0],
        this->signals[2],
        this->dspexpr_03_in3,
        this->signals[1],
        n
    );

    this->dspexpr_02_perform(this->signals[1], this->dspexpr_02_in2, this->signals[2], n);
    this->gen_01_perform(this->signals[2], this->gen_01_in2, this->gen_01_in3, this->signals[1], n);
    this->dspexpr_01_perform(this->signals[1], this->dspexpr_01_in2, this->signals[2], n);

    this->adsr_01_perform(
        this->adsr_01_attack,
        this->adsr_01_decay,
        this->adsr_01_sustain,
        this->adsr_01_release,
        this->zeroBuffer,
        this->signals[1],
        n
    );

    this->noise_tilde_02_perform(this->signals[0], n);
    this->dspexpr_06_perform(this->signals[1], this->signals[0], this->signals[3], n);
    this->dspexpr_05_perform(this->signals[3], this->dspexpr_05_in2, this->signals[0], n);
    this->signaladder_01_perform(this->signals[2], this->signals[0], out1, n);

    this->adsr_02_perform(
        this->adsr_02_attack,
        this->adsr_02_decay,
        this->adsr_02_sustain,
        this->adsr_02_release,
        this->zeroBuffer,
        this->signals[0],
        n
    );

    this->noise_tilde_03_perform(this->signals[3], n);
    this->dspexpr_08_perform(this->signals[0], this->signals[3], this->signals[1], n);
    this->dspexpr_07_perform(this->signals[1], this->dspexpr_07_in2, this->signals[3], n);
    this->signaladder_02_perform(this->signals[2], this->signals[3], out2, n);
    this->stackprotect_perform(n);
    this->globaltransport_advance();
    this->advanceTime((ENGINE*)nullptr);
    this->audioProcessSampleCount += this->vs;
}

void prepareToProcess(number sampleRate, Index maxBlockSize, bool force) {
    RNBO_ASSERT(this->_isInitialized);

    if (this->maxvs < maxBlockSize || !this->didAllocateSignals) {
        Index i;

        for (i = 0; i < 4; i++) {
            this->signals[i] = resizeSignal(this->signals[i], this->maxvs, maxBlockSize);
        }

        this->adsr_01_triggerBuf = resizeSignal(this->adsr_01_triggerBuf, this->maxvs, maxBlockSize);
        this->adsr_01_triggerValueBuf = resizeSignal(this->adsr_01_triggerValueBuf, this->maxvs, maxBlockSize);
        this->adsr_02_triggerBuf = resizeSignal(this->adsr_02_triggerBuf, this->maxvs, maxBlockSize);
        this->adsr_02_triggerValueBuf = resizeSignal(this->adsr_02_triggerValueBuf, this->maxvs, maxBlockSize);
        this->globaltransport_tempo = resizeSignal(this->globaltransport_tempo, this->maxvs, maxBlockSize);
        this->globaltransport_state = resizeSignal(this->globaltransport_state, this->maxvs, maxBlockSize);
        this->zeroBuffer = resizeSignal(this->zeroBuffer, this->maxvs, maxBlockSize);
        this->dummyBuffer = resizeSignal(this->dummyBuffer, this->maxvs, maxBlockSize);
        this->didAllocateSignals = true;
    }

    const bool sampleRateChanged = sampleRate != this->sr;
    const bool maxvsChanged = maxBlockSize != this->maxvs;
    const bool forceDSPSetup = sampleRateChanged || maxvsChanged || force;

    if (sampleRateChanged || maxvsChanged) {
        this->vs = maxBlockSize;
        this->maxvs = maxBlockSize;
        this->sr = sampleRate;
        this->invsr = 1 / sampleRate;
    }

    this->noise_tilde_01_dspsetup(forceDSPSetup);
    this->adsr_01_dspsetup(forceDSPSetup);
    this->noise_tilde_02_dspsetup(forceDSPSetup);
    this->adsr_02_dspsetup(forceDSPSetup);
    this->noise_tilde_03_dspsetup(forceDSPSetup);
    this->globaltransport_dspsetup(forceDSPSetup);

    if (sampleRateChanged)
        this->onSampleRateChanged(sampleRate);
}

number msToSamps(MillisecondTime ms, number sampleRate) {
    return ms * sampleRate * 0.001;
}

MillisecondTime sampsToMs(SampleIndex samps) {
    return samps * (this->invsr * 1000);
}

Index getNumInputChannels() const {
    return 0;
}

Index getNumOutputChannels() const {
    return 2;
}

DataRef* getDataRef(DataRefIndex index)  {
    switch (index) {
    case 0:
        {
        return addressOf(this->pink_tilde_01_rowsobj);
        break;
        }
    default:
        {
        return nullptr;
        }
    }
}

DataRefIndex getNumDataRefs() const {
    return 1;
}

void processDataViewUpdate(DataRefIndex index, MillisecondTime time) {
    this->updateTime(time, (ENGINE*)nullptr);

    if (index == 0) {
        this->pink_tilde_01_rows = reInitDataView(this->pink_tilde_01_rows, this->pink_tilde_01_rowsobj);
    }
}

void initialize() {
    RNBO_ASSERT(!this->_isInitialized);

    this->pink_tilde_01_rowsobj = initDataRef(
        this->pink_tilde_01_rowsobj,
        this->dataRefStrings->name0,
        true,
        this->dataRefStrings->file0,
        this->dataRefStrings->tag0
    );

    this->assign_defaults();
    this->applyState();
    this->pink_tilde_01_rowsobj->setIndex(0);
    this->pink_tilde_01_rows = new IntBuffer(this->pink_tilde_01_rowsobj);
    this->initializeObjects();
    this->allocateDataRefs();
    this->startup();
    this->_isInitialized = true;
}

void getPreset(PatcherStateInterface& preset) {
    this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr);
    preset["__presetid"] = "rnbo";
    this->param_01_getPresetValue(getSubState(preset, "noisewarmth"));
    this->param_02_getPresetValue(getSubState(preset, "noisebrightness"));
    this->param_03_getPresetValue(getSubState(preset, "noisevolume"));
    this->param_04_getPresetValue(getSubState(preset, "cracklefrequency"));
    this->param_05_getPresetValue(getSubState(preset, "cracklevolume"));
}

void setPreset(MillisecondTime time, PatcherStateInterface& preset) {
    this->updateTime(time, (ENGINE*)nullptr);
    this->param_01_setPresetValue(getSubState(preset, "noisewarmth"));
    this->param_02_setPresetValue(getSubState(preset, "noisebrightness"));
    this->param_03_setPresetValue(getSubState(preset, "noisevolume"));
    this->param_04_setPresetValue(getSubState(preset, "cracklefrequency"));
    this->param_05_setPresetValue(getSubState(preset, "cracklevolume"));
}

void setParameterValue(ParameterIndex index, ParameterValue v, MillisecondTime time) {
    this->updateTime(time, (ENGINE*)nullptr);

    switch (index) {
    case 0:
        {
        this->param_01_value_set(v);
        break;
        }
    case 1:
        {
        this->param_02_value_set(v);
        break;
        }
    case 2:
        {
        this->param_03_value_set(v);
        break;
        }
    case 3:
        {
        this->param_04_value_set(v);
        break;
        }
    case 4:
        {
        this->param_05_value_set(v);
        break;
        }
    }
}

void processParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
    this->setParameterValue(index, value, time);
}

void processParameterBangEvent(ParameterIndex index, MillisecondTime time) {
    this->setParameterValue(index, this->getParameterValue(index), time);
}

void processNormalizedParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
    this->setParameterValueNormalized(index, value, time);
}

ParameterValue getParameterValue(ParameterIndex index)  {
    switch (index) {
    case 0:
        {
        return this->param_01_value;
        }
    case 1:
        {
        return this->param_02_value;
        }
    case 2:
        {
        return this->param_03_value;
        }
    case 3:
        {
        return this->param_04_value;
        }
    case 4:
        {
        return this->param_05_value;
        }
    default:
        {
        return 0;
        }
    }
}

ParameterIndex getNumSignalInParameters() const {
    return 0;
}

ParameterIndex getNumSignalOutParameters() const {
    return 0;
}

ParameterIndex getNumParameters() const {
    return 5;
}

ConstCharPointer getParameterName(ParameterIndex index) const {
    switch (index) {
    case 0:
        {
        return "noisewarmth";
        }
    case 1:
        {
        return "noisebrightness";
        }
    case 2:
        {
        return "noisevolume";
        }
    case 3:
        {
        return "cracklefrequency";
        }
    case 4:
        {
        return "cracklevolume";
        }
    default:
        {
        return "bogus";
        }
    }
}

ConstCharPointer getParameterId(ParameterIndex index) const {
    switch (index) {
    case 0:
        {
        return "noisewarmth";
        }
    case 1:
        {
        return "noisebrightness";
        }
    case 2:
        {
        return "noisevolume";
        }
    case 3:
        {
        return "cracklefrequency";
        }
    case 4:
        {
        return "cracklevolume";
        }
    default:
        {
        return "bogus";
        }
    }
}

void getParameterInfo(ParameterIndex index, ParameterInfo * info) const {
    {
        switch (index) {
        case 0:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 1:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 64;
            info->min = 36;
            info->max = 127;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 2:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = -10;
            info->min = -60;
            info->max = 0;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 3:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0.5;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 4:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = -10;
            info->min = -60;
            info->max = 0;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        }
    }
}

ParameterValue applyStepsToNormalizedParameterValue(ParameterValue normalizedValue, int steps) const {
    if (steps == 1) {
        if (normalizedValue > 0) {
            normalizedValue = 1.;
        }
    } else {
        ParameterValue oneStep = (number)1. / (steps - 1);
        ParameterValue numberOfSteps = rnbo_fround(normalizedValue / oneStep * 1 / (number)1) * (number)1;
        normalizedValue = numberOfSteps * oneStep;
    }

    return normalizedValue;
}

ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
    switch (index) {
    case 0:
    case 3:
        {
        {
            value = (value < 0 ? 0 : (value > 1 ? 1 : value));
            ParameterValue normalizedValue = (value - 0) / (1 - 0);
            return normalizedValue;
        }
        }
    case 1:
        {
        {
            value = (value < 36 ? 36 : (value > 127 ? 127 : value));
            ParameterValue normalizedValue = (value - 36) / (127 - 36);
            return normalizedValue;
        }
        }
    case 2:
    case 4:
        {
        {
            value = (value < -60 ? -60 : (value > 0 ? 0 : value));
            ParameterValue normalizedValue = (value - -60) / (0 - -60);
            return normalizedValue;
        }
        }
    default:
        {
        return value;
        }
    }
}

ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
    value = (value < 0 ? 0 : (value > 1 ? 1 : value));

    switch (index) {
    case 0:
    case 3:
        {
        {
            {
                return 0 + value * (1 - 0);
            }
        }
        }
    case 1:
        {
        {
            {
                return 36 + value * (127 - 36);
            }
        }
        }
    case 2:
    case 4:
        {
        {
            {
                return -60 + value * (0 - -60);
            }
        }
        }
    default:
        {
        return value;
        }
    }
}

ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const {
    switch (index) {
    case 0:
        {
        return this->param_01_value_constrain(value);
        }
    case 1:
        {
        return this->param_02_value_constrain(value);
        }
    case 2:
        {
        return this->param_03_value_constrain(value);
        }
    case 3:
        {
        return this->param_04_value_constrain(value);
        }
    case 4:
        {
        return this->param_05_value_constrain(value);
        }
    default:
        {
        return value;
        }
    }
}

void processNumMessage(MessageTag tag, MessageTag objectId, MillisecondTime time, number payload) {
    this->updateTime(time, (ENGINE*)nullptr);

    switch (tag) {
    case TAG("valin"):
        {
        if (TAG("number_obj-288") == objectId)
            this->numberobj_01_valin_set(payload);

        if (TAG("number_obj-307") == objectId)
            this->numberobj_02_valin_set(payload);

        if (TAG("number_obj-312") == objectId)
            this->numberobj_03_valin_set(payload);

        if (TAG("number_obj-281") == objectId)
            this->numberobj_04_valin_set(payload);

        if (TAG("number_obj-369") == objectId)
            this->numberobj_05_valin_set(payload);

        if (TAG("number_obj-83") == objectId)
            this->numberobj_06_valin_set(payload);

        if (TAG("number_obj-135") == objectId)
            this->numberobj_07_valin_set(payload);

        if (TAG("number_obj-209") == objectId)
            this->numberobj_08_valin_set(payload);

        if (TAG("number_obj-370") == objectId)
            this->numberobj_09_valin_set(payload);

        break;
        }
    case TAG("format"):
        {
        if (TAG("number_obj-288") == objectId)
            this->numberobj_01_format_set(payload);

        if (TAG("number_obj-307") == objectId)
            this->numberobj_02_format_set(payload);

        if (TAG("number_obj-312") == objectId)
            this->numberobj_03_format_set(payload);

        if (TAG("number_obj-281") == objectId)
            this->numberobj_04_format_set(payload);

        if (TAG("number_obj-369") == objectId)
            this->numberobj_05_format_set(payload);

        if (TAG("number_obj-83") == objectId)
            this->numberobj_06_format_set(payload);

        if (TAG("number_obj-135") == objectId)
            this->numberobj_07_format_set(payload);

        if (TAG("number_obj-209") == objectId)
            this->numberobj_08_format_set(payload);

        if (TAG("number_obj-370") == objectId)
            this->numberobj_09_format_set(payload);

        break;
        }
    }
}

void processListMessage(MessageTag , MessageTag , MillisecondTime , const list& ) {}

void processBangMessage(MessageTag tag, MessageTag objectId, MillisecondTime time) {
    this->updateTime(time, (ENGINE*)nullptr);

    switch (tag) {
    case TAG("startupbang"):
        {
        if (TAG("loadbang_obj-25") == objectId)
            this->loadbang_01_startupbang_bang();

        if (TAG("loadbang_obj-7") == objectId)
            this->loadbang_02_startupbang_bang();

        if (TAG("loadbang_obj-76") == objectId)
            this->loadbang_03_startupbang_bang();

        if (TAG("loadbang_obj-289") == objectId)
            this->loadbang_04_startupbang_bang();

        break;
        }
    case TAG("bangin"):
        {
        if (TAG("button_obj-14") == objectId)
            this->button_01_bangin_bang();

        if (TAG("button_obj-75") == objectId)
            this->button_02_bangin_bang();

        if (TAG("button_obj-329") == objectId)
            this->button_03_bangin_bang();

        break;
        }
    }
}

MessageTagInfo resolveTag(MessageTag tag) const {
    switch (tag) {
    case TAG("valout"):
        {
        return "valout";
        }
    case TAG("number_obj-288"):
        {
        return "number_obj-288";
        }
    case TAG("setup"):
        {
        return "setup";
        }
    case TAG("number_obj-307"):
        {
        return "number_obj-307";
        }
    case TAG("number_obj-312"):
        {
        return "number_obj-312";
        }
    case TAG("bangout"):
        {
        return "bangout";
        }
    case TAG("button_obj-14"):
        {
        return "button_obj-14";
        }
    case TAG("number_obj-281"):
        {
        return "number_obj-281";
        }
    case TAG("number_obj-369"):
        {
        return "number_obj-369";
        }
    case TAG("button_obj-75"):
        {
        return "button_obj-75";
        }
    case TAG("number_obj-83"):
        {
        return "number_obj-83";
        }
    case TAG("number_obj-135"):
        {
        return "number_obj-135";
        }
    case TAG("button_obj-329"):
        {
        return "button_obj-329";
        }
    case TAG("number_obj-209"):
        {
        return "number_obj-209";
        }
    case TAG("number_obj-370"):
        {
        return "number_obj-370";
        }
    case TAG("valin"):
        {
        return "valin";
        }
    case TAG("format"):
        {
        return "format";
        }
    case TAG("startupbang"):
        {
        return "startupbang";
        }
    case TAG("loadbang_obj-25"):
        {
        return "loadbang_obj-25";
        }
    case TAG("loadbang_obj-7"):
        {
        return "loadbang_obj-7";
        }
    case TAG("bangin"):
        {
        return "bangin";
        }
    case TAG("loadbang_obj-76"):
        {
        return "loadbang_obj-76";
        }
    case TAG("loadbang_obj-289"):
        {
        return "loadbang_obj-289";
        }
    }

    return "";
}

MessageIndex getNumMessages() const {
    return 0;
}

const MessageInfo& getMessageInfo(MessageIndex index) const {
    switch (index) {

    }

    return NullMessageInfo;
}

protected:

		
void advanceTime(EXTERNALENGINE*) {}
void advanceTime(INTERNALENGINE*) {
	_internalEngine.advanceTime(sampstoms(this->vs));
}

void processInternalEvents(MillisecondTime time) {
	_internalEngine.processEventsUntil(time);
}

void updateTime(MillisecondTime time, INTERNALENGINE*, bool inProcess = false) {
	if (time == TimeNow) time = getPatcherTime();
	processInternalEvents(inProcess ? time + sampsToMs(this->vs) : time);
	updateTime(time, (EXTERNALENGINE*)nullptr);
}

rnbomatic* operator->() {
    return this;
}
const rnbomatic* operator->() const {
    return this;
}
rnbomatic* getTopLevelPatcher() {
    return this;
}

void cancelClockEvents()
{
    getEngine()->flushClockEvents(this, -871642103, false);
    getEngine()->flushClockEvents(this, -1735137014, false);
    getEngine()->flushClockEvents(this, 832840460, false);
    getEngine()->flushClockEvents(this, -217479832, false);
    getEngine()->flushClockEvents(this, 1696335371, false);
    getEngine()->flushClockEvents(this, -1357044121, false);
    getEngine()->flushClockEvents(this, -1096817259, false);
    getEngine()->flushClockEvents(this, -1468824490, false);
}

template<typename LISTTYPE = list> void listquicksort(LISTTYPE& arr, LISTTYPE& sortindices, Int l, Int h, bool ascending) {
    if (l < h) {
        Int p = (Int)(this->listpartition(arr, sortindices, l, h, ascending));
        this->listquicksort(arr, sortindices, l, p - 1, ascending);
        this->listquicksort(arr, sortindices, p + 1, h, ascending);
    }
}

template<typename LISTTYPE = list> Int listpartition(LISTTYPE& arr, LISTTYPE& sortindices, Int l, Int h, bool ascending) {
    number x = arr[(Index)h];
    Int i = (Int)(l - 1);

    for (Int j = (Int)(l); j <= h - 1; j++) {
        bool asc = (bool)((bool)(ascending) && arr[(Index)j] <= x);
        bool desc = (bool)((bool)(!(bool)(ascending)) && arr[(Index)j] >= x);

        if ((bool)(asc) || (bool)(desc)) {
            i++;
            this->listswapelements(arr, i, j);
            this->listswapelements(sortindices, i, j);
        }
    }

    i++;
    this->listswapelements(arr, i, h);
    this->listswapelements(sortindices, i, h);
    return i;
}

template<typename LISTTYPE = list> void listswapelements(LISTTYPE& arr, Int a, Int b) {
    auto tmp = arr[(Index)a];
    arr[(Index)a] = arr[(Index)b];
    arr[(Index)b] = tmp;
}

Index voice() {
    return this->_voiceIndex;
}

number random(number low, number high) {
    number range = high - low;
    return globalrandom() * range + low;
}

number maximum(number x, number y) {
    return (x < y ? y : x);
}

inline number safediv(number num, number denom) {
    return (denom == 0.0 ? 0.0 : num / denom);
}

number safepow(number base, number exponent) {
    return fixnan(rnbo_pow(base, exponent));
}

number scale(
    number x,
    number lowin,
    number hiin,
    number lowout,
    number highout,
    number pow
) {
    auto inscale = this->safediv(1., hiin - lowin);
    number outdiff = highout - lowout;
    number value = (x - lowin) * inscale;

    if (pow != 1) {
        if (value > 0)
            value = this->safepow(value, pow);
        else
            value = -this->safepow(-value, pow);
    }

    value = value * outdiff + lowout;
    return value;
}

number mstosamps(MillisecondTime ms) {
    return ms * this->sr * 0.001;
}

MillisecondTime sampstoms(number samps) {
    return samps * 1000 / this->sr;
}

void param_01_value_set(number v) {
    v = this->param_01_value_constrain(v);
    this->param_01_value = v;
    this->sendParameter(0, false);

    if (this->param_01_value != this->param_01_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_01_lastValue = this->param_01_value;
    }

    this->send_01_input_number_set(v);
}

void param_02_value_set(number v) {
    v = this->param_02_value_constrain(v);
    this->param_02_value = v;
    this->sendParameter(1, false);

    if (this->param_02_value != this->param_02_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_02_lastValue = this->param_02_value;
    }

    this->send_02_input_number_set(v);
}

void param_03_value_set(number v) {
    v = this->param_03_value_constrain(v);
    this->param_03_value = v;
    this->sendParameter(2, false);

    if (this->param_03_value != this->param_03_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_03_lastValue = this->param_03_value;
    }

    this->send_03_input_number_set(v);
}

void param_04_value_set(number v) {
    v = this->param_04_value_constrain(v);
    this->param_04_value = v;
    this->sendParameter(3, false);

    if (this->param_04_value != this->param_04_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_04_lastValue = this->param_04_value;
    }

    this->send_04_input_number_set(v);
}

void param_05_value_set(number v) {
    v = this->param_05_value_constrain(v);
    this->param_05_value = v;
    this->sendParameter(4, false);

    if (this->param_05_value != this->param_05_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_05_lastValue = this->param_05_value;
    }

    this->send_05_input_number_set(v);
}

MillisecondTime getPatcherTime() const {
    return this->_currentTime;
}

void numberobj_01_valin_set(number v) {
    this->numberobj_01_value_set(v);
}

void numberobj_01_format_set(number v) {
    this->numberobj_01_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void loadbang_01_startupbang_bang() {
    this->loadbang_01_output_bang();
}

void numberobj_02_valin_set(number v) {
    this->numberobj_02_value_set(v);
}

void numberobj_02_format_set(number v) {
    this->numberobj_02_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void loadbang_02_startupbang_bang() {
    this->loadbang_02_output_bang();
}

void numberobj_03_valin_set(number v) {
    this->numberobj_03_value_set(v);
}

void numberobj_03_format_set(number v) {
    this->numberobj_03_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void button_01_bangin_bang() {
    this->button_01_bangval_bang();
}

void numberobj_04_valin_set(number v) {
    this->numberobj_04_value_set(v);
}

void numberobj_04_format_set(number v) {
    this->numberobj_04_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void numberobj_05_valin_set(number v) {
    this->numberobj_05_value_set(v);
}

void numberobj_05_format_set(number v) {
    this->numberobj_05_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void loadbang_03_startupbang_bang() {
    this->loadbang_03_output_bang();
}

void button_02_bangin_bang() {
    this->button_02_bangval_bang();
}

void delay_01_out_bang() {
    this->button_02_bangval_bang();
}

void numberobj_06_valin_set(number v) {
    this->numberobj_06_value_set(v);
}

void numberobj_06_format_set(number v) {
    this->numberobj_06_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void numberobj_07_valin_set(number v) {
    this->numberobj_07_value_set(v);
}

void numberobj_07_format_set(number v) {
    this->numberobj_07_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void loadbang_04_startupbang_bang() {
    this->loadbang_04_output_bang();
}

void button_03_bangin_bang() {
    this->button_03_bangval_bang();
}

void delay_02_out_bang() {
    this->button_03_bangval_bang();
}

void numberobj_08_valin_set(number v) {
    this->numberobj_08_value_set(v);
}

void numberobj_08_format_set(number v) {
    this->numberobj_08_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void numberobj_09_valin_set(number v) {
    this->numberobj_09_value_set(v);
}

void numberobj_09_format_set(number v) {
    this->numberobj_09_currentFormat = trunc((v > 6 ? 6 : (v < 0 ? 0 : v)));
}

void adsr_01_mute_bang() {}

void adsr_02_mute_bang() {}

void deallocateSignals() {
    Index i;

    for (i = 0; i < 4; i++) {
        this->signals[i] = freeSignal(this->signals[i]);
    }

    this->adsr_01_triggerBuf = freeSignal(this->adsr_01_triggerBuf);
    this->adsr_01_triggerValueBuf = freeSignal(this->adsr_01_triggerValueBuf);
    this->adsr_02_triggerBuf = freeSignal(this->adsr_02_triggerBuf);
    this->adsr_02_triggerValueBuf = freeSignal(this->adsr_02_triggerValueBuf);
    this->globaltransport_tempo = freeSignal(this->globaltransport_tempo);
    this->globaltransport_state = freeSignal(this->globaltransport_state);
    this->zeroBuffer = freeSignal(this->zeroBuffer);
    this->dummyBuffer = freeSignal(this->dummyBuffer);
}

Index getMaxBlockSize() const {
    return this->maxvs;
}

number getSampleRate() const {
    return this->sr;
}

bool hasFixedVectorSize() const {
    return false;
}

void setProbingTarget(MessageTag ) {}

void fillDataRef(DataRefIndex , DataRef& ) {}

void zeroDataRef(DataRef& ref) {
    ref->setZero();
}

void allocateDataRefs() {
    this->pink_tilde_01_rows->requestSize(16);
    this->pink_tilde_01_rows = this->pink_tilde_01_rows->allocateIfNeeded();

    if (this->pink_tilde_01_rowsobj->hasRequestedSize()) {
        if (this->pink_tilde_01_rowsobj->wantsFill())
            this->zeroDataRef(this->pink_tilde_01_rowsobj);

        this->getEngine()->sendDataRefUpdated(0);
    }
}

void initializeObjects() {
    this->numberobj_01_init();
    this->noise_tilde_01_nz_init();
    this->gen_01_history_1_init();
    this->gen_01_history_2_init();
    this->gen_01_history_3_init();
    this->gen_01_history_4_init();
    this->numberobj_02_init();
    this->pink_tilde_01_init();
    this->numberobj_03_init();
    this->numberobj_04_init();
    this->numberobj_05_init();
    this->noise_tilde_02_nz_init();
    this->numberobj_06_init();
    this->random_01_nz_init();
    this->random_02_nz_init();
    this->numberobj_07_init();
    this->noise_tilde_03_nz_init();
    this->numberobj_08_init();
    this->random_03_nz_init();
    this->random_04_nz_init();
    this->numberobj_09_init();
}

Index getIsMuted()  {
    return this->isMuted;
}

void setIsMuted(Index v)  {
    this->isMuted = v;
}

void onSampleRateChanged(double ) {}

void extractState(PatcherStateInterface& ) {}

void applyState() {}

void processClockEvent(MillisecondTime time, ClockId index, bool hasValue, ParameterValue value) {
    RNBO_UNUSED(value);
    RNBO_UNUSED(hasValue);
    this->updateTime(time, (ENGINE*)nullptr);

    switch (index) {
    case -871642103:
        {
        this->loadbang_01_startupbang_bang();
        break;
        }
    case -1735137014:
        {
        this->loadbang_02_startupbang_bang();
        break;
        }
    case 832840460:
        {
        this->loadbang_03_startupbang_bang();
        break;
        }
    case -217479832:
        {
        this->delay_01_out_bang();
        break;
        }
    case 1696335371:
        {
        this->loadbang_04_startupbang_bang();
        break;
        }
    case -1357044121:
        {
        this->delay_02_out_bang();
        break;
        }
    case -1096817259:
        {
        this->adsr_01_mute_bang();
        break;
        }
    case -1468824490:
        {
        this->adsr_02_mute_bang();
        break;
        }
    }
}

void processOutletAtCurrentTime(EngineLink* , OutletIndex , ParameterValue ) {}

void processOutletEvent(
    EngineLink* sender,
    OutletIndex index,
    ParameterValue value,
    MillisecondTime time
) {
    this->updateTime(time, (ENGINE*)nullptr);
    this->processOutletAtCurrentTime(sender, index, value);
}

void sendOutlet(OutletIndex index, ParameterValue value) {
    this->getEngine()->sendOutlet(this, index, value);
}

void startup() {
    this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr);
    this->getEngine()->scheduleClockEvent(this, -871642103, 0 + this->_currentTime);;
    this->getEngine()->scheduleClockEvent(this, -1735137014, 0 + this->_currentTime);;
    this->getEngine()->scheduleClockEvent(this, 832840460, 0 + this->_currentTime);;

    {
        this->random_02_nz_seed(this->random_02_seed);
    }

    this->getEngine()->scheduleClockEvent(this, 1696335371, 0 + this->_currentTime);;

    {
        this->random_04_nz_seed(this->random_04_seed);
    }

    {
        this->scheduleParamInit(0, 0);
    }

    {
        this->scheduleParamInit(1, 0);
    }

    {
        this->scheduleParamInit(2, 0);
    }

    {
        this->scheduleParamInit(3, 0);
    }

    {
        this->scheduleParamInit(4, 0);
    }

    this->processParamInitEvents();
}

number param_01_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

void dspexpr_03_in3_set(number v) {
    this->dspexpr_03_in3 = v;
}

void numberobj_04_output_set(number v) {
    this->dspexpr_03_in3_set(v);
}

void numberobj_04_value_set(number v) {
    this->numberobj_04_value_setter(v);
    v = this->numberobj_04_value;
    number localvalue = v;

    if ((bool)(true) && localvalue < 0) {
        localvalue = 0;
    }

    if ((bool)(true) && localvalue > 1) {
        localvalue = 1;
    }

    if (this->numberobj_04_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_04_output_set(localvalue);
}

void receive_02_output_number_set(number v) {
    this->receive_02_output_number = v;
    this->numberobj_04_value_set(v);
}

void send_01_input_number_set(number v) {
    this->send_01_input_number = v;
    this->receive_02_output_number_set(v);
}

number param_02_value_constrain(number v) const {
    v = (v > 127 ? 127 : (v < 36 ? 36 : v));
    return v;
}

void gen_01_in3_set(number v) {
    this->gen_01_in3 = v;
}

template<typename LISTTYPE> void scale_01_out_set(const LISTTYPE& v) {
    this->scale_01_out = jsCreateListCopy(v);

    {
        number converted = (v->length > 0 ? v[0] : 0);
        this->gen_01_in3_set(converted);
    }
}

template<typename LISTTYPE> void scale_01_input_set(const LISTTYPE& v) {
    this->scale_01_input = jsCreateListCopy(v);
    list tmp = {};

    for (Index i = 0; i < v->length; i++) {
        tmp->push(this->scale(
            v[(Index)i],
            this->scale_01_inlow,
            this->scale_01_inhigh,
            this->scale_01_outlow,
            this->scale_01_outhigh,
            this->scale_01_power
        ));
    }

    this->scale_01_out_set(tmp);
}

void receive_03_output_number_set(number v) {
    this->receive_03_output_number = v;

    {
        listbase<number, 1> converted = {v};
        this->scale_01_input_set(converted);
    }
}

void send_02_input_number_set(number v) {
    this->send_02_input_number = v;
    this->receive_03_output_number_set(v);
}

number param_03_value_constrain(number v) const {
    v = (v > 0 ? 0 : (v < -60 ? -60 : v));
    return v;
}

void dspexpr_01_in2_set(number v) {
    this->dspexpr_01_in2 = v;
}

void numberobj_03_output_set(number v) {
    this->dspexpr_01_in2_set(v);
}

void numberobj_03_value_set(number v) {
    this->numberobj_03_value_setter(v);
    v = this->numberobj_03_value;
    number localvalue = v;

    if (this->numberobj_03_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_03_output_set(localvalue);
}

void expr_01_out1_set(number v) {
    this->expr_01_out1 = v;
    this->numberobj_03_value_set(this->expr_01_out1);
}

void expr_01_in1_set(number in1) {
    this->expr_01_in1 = in1;
    this->expr_01_out1_set(rnbo_pow(10, this->expr_01_in1 * 0.05));//#map:dbtoa_obj-311:1
}

void receive_01_output_number_set(number v) {
    this->receive_01_output_number = v;
    this->expr_01_in1_set(v);
}

void send_03_input_number_set(number v) {
    this->send_03_input_number = v;
    this->receive_01_output_number_set(v);
}

number param_04_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

void random_01_limit_set(number v) {
    this->random_01_limit_setter(v);
    v = this->random_01_limit;
}

template<typename LISTTYPE> void scale_02_out_set(const LISTTYPE& v) {
    this->scale_02_out = jsCreateListCopy(v);

    {
        number converted = (v->length > 0 ? v[0] : 0);
        this->random_01_limit_set(converted);
    }
}

template<typename LISTTYPE> void scale_02_input_set(const LISTTYPE& v) {
    this->scale_02_input = jsCreateListCopy(v);
    list tmp = {};

    for (Index i = 0; i < v->length; i++) {
        tmp->push(this->scale(
            v[(Index)i],
            this->scale_02_inlow,
            this->scale_02_inhigh,
            this->scale_02_outlow,
            this->scale_02_outhigh,
            this->scale_02_power
        ));
    }

    this->scale_02_out_set(tmp);
}

void receive_05_output_number_set(number v) {
    this->receive_05_output_number = v;

    {
        listbase<number, 1> converted = {v};
        this->scale_02_input_set(converted);
    }
}

void random_03_limit_set(number v) {
    this->random_03_limit_setter(v);
    v = this->random_03_limit;
}

template<typename LISTTYPE> void scale_03_out_set(const LISTTYPE& v) {
    this->scale_03_out = jsCreateListCopy(v);

    {
        number converted = (v->length > 0 ? v[0] : 0);
        this->random_03_limit_set(converted);
    }
}

template<typename LISTTYPE> void scale_03_input_set(const LISTTYPE& v) {
    this->scale_03_input = jsCreateListCopy(v);
    list tmp = {};

    for (Index i = 0; i < v->length; i++) {
        tmp->push(this->scale(
            v[(Index)i],
            this->scale_03_inlow,
            this->scale_03_inhigh,
            this->scale_03_outlow,
            this->scale_03_outhigh,
            this->scale_03_power
        ));
    }

    this->scale_03_out_set(tmp);
}

void receive_07_output_number_set(number v) {
    this->receive_07_output_number = v;

    {
        listbase<number, 1> converted = {v};
        this->scale_03_input_set(converted);
    }
}

void send_04_input_number_set(number v) {
    this->send_04_input_number = v;
    this->receive_05_output_number_set(v);
    this->receive_07_output_number_set(v);
}

number param_05_value_constrain(number v) const {
    v = (v > 0 ? 0 : (v < -60 ? -60 : v));
    return v;
}

void dspexpr_05_in2_set(number v) {
    this->dspexpr_05_in2 = v;
}

void numberobj_06_output_set(number v) {
    this->dspexpr_05_in2_set(v);
}

void numberobj_06_value_set(number v) {
    this->numberobj_06_value_setter(v);
    v = this->numberobj_06_value;
    number localvalue = v;

    if (this->numberobj_06_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_06_output_set(localvalue);
}

void expr_02_out1_set(number v) {
    this->expr_02_out1 = v;
    this->numberobj_06_value_set(this->expr_02_out1);
}

void expr_02_in1_set(number in1) {
    this->expr_02_in1 = in1;
    this->expr_02_out1_set(rnbo_pow(10, this->expr_02_in1 * 0.05));//#map:dbtoa_obj-85:1
}

void receive_04_output_number_set(number v) {
    this->receive_04_output_number = v;
    this->expr_02_in1_set(v);
}

void dspexpr_07_in2_set(number v) {
    this->dspexpr_07_in2 = v;
}

void numberobj_08_output_set(number v) {
    this->dspexpr_07_in2_set(v);
}

void numberobj_08_value_set(number v) {
    this->numberobj_08_value_setter(v);
    v = this->numberobj_08_value;
    number localvalue = v;

    if (this->numberobj_08_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_08_output_set(localvalue);
}

void expr_04_out1_set(number v) {
    this->expr_04_out1 = v;
    this->numberobj_08_value_set(this->expr_04_out1);
}

void expr_04_in1_set(number in1) {
    this->expr_04_in1 = in1;
    this->expr_04_out1_set(rnbo_pow(10, this->expr_04_in1 * 0.05));//#map:dbtoa_obj-211:1
}

void receive_06_output_number_set(number v) {
    this->receive_06_output_number = v;
    this->expr_04_in1_set(v);
}

void send_05_input_number_set(number v) {
    this->send_05_input_number = v;
    this->receive_04_output_number_set(v);
    this->receive_06_output_number_set(v);
}

void numberobj_01_output_set(number v) {
    this->param_01_value_set(v);
}

void numberobj_01_value_set(number v) {
    this->numberobj_01_value_setter(v);
    v = this->numberobj_01_value;
    number localvalue = v;

    if ((bool)(true) && localvalue < 0) {
        localvalue = 0;
    }

    if ((bool)(true) && localvalue > 1) {
        localvalue = 1;
    }

    if (this->numberobj_01_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_01_output_set(localvalue);
}

void noise_tilde_01_input_bang() {}

void loadbang_01_output_bang() {
    this->noise_tilde_01_input_bang();
}

void numberobj_02_output_set(number v) {
    this->param_02_value_set(v);
}

void numberobj_02_value_set(number v) {
    this->numberobj_02_value_setter(v);
    v = this->numberobj_02_value;
    number localvalue = v;

    if ((bool)(true) && localvalue < 36) {
        localvalue = 36;
    }

    if ((bool)(true) && localvalue > 127) {
        localvalue = 127;
    }

    if (this->numberobj_02_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_02_output_set(localvalue);
}

void gen_01_in2_set(number v) {
    this->gen_01_in2 = v;
}

void floatnum_01_out_set(number v) {
    this->gen_01_in2_set(v);
}

void floatnum_01_input_bang_bang() {
    this->floatnum_01_out_set(this->floatnum_01_stored);
}

void loadbang_02_output_bang() {
    this->floatnum_01_input_bang_bang();
}

void button_01_output_bang() {
    this->floatnum_01_input_bang_bang();
}

void button_01_bangval_bang() {
    ;
    this->button_01_output_bang();
}

void numberobj_05_output_set(number v) {
    this->param_03_value_set(v);
}

void numberobj_05_value_set(number v) {
    this->numberobj_05_value_setter(v);
    v = this->numberobj_05_value;
    number localvalue = v;

    if ((bool)(true) && localvalue < -60) {
        localvalue = -60;
    }

    if ((bool)(true) && localvalue > 0) {
        localvalue = 0;
    }

    if (this->numberobj_05_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_05_output_set(localvalue);
}

void expr_03_in2_set(number v) {
    this->expr_03_in2 = v;
}

void random_02_result_set(number v) {
    this->expr_03_in2_set(v);
}

void random_02_trigger_bang() {
    this->random_02_result_set(rnbo_abs(this->random_02_nz_next()) * this->random_02_limit);
}

number delay_01_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void delay_01_time_set(number v) {
    v = this->delay_01_time_constrain(v);
    this->delay_01_time = v;
}

void expr_03_out1_set(number v) {
    this->expr_03_out1 = v;
    this->delay_01_time_set(this->expr_03_out1);
}

void expr_03_in1_set(number in1) {
    this->expr_03_in1 = in1;
    this->expr_03_out1_set(this->expr_03_in1 + this->expr_03_in2);//#map:+_obj-19:1
}

void random_01_result_set(number v) {
    this->expr_03_in1_set(v);
}

void random_01_trigger_bang() {
    this->random_01_result_set(rnbo_abs(this->random_01_nz_next()) * this->random_01_limit);
}

void delay_01_stop_bang() {
    this->getEngine()->flushClockEvents(this, -217479832, false);;
}

void delay_01_input_bang() {
    if ((bool)(!(bool)(this->delay_01_delayall)))
        this->delay_01_stop_bang();

    this->getEngine()->scheduleClockEvent(this, -217479832, this->delay_01_time + this->_currentTime);;
}

void adsr_01_trigger_number_set(number v) {
    this->adsr_01_trigger_number = v;

    if (v != 0)
        this->adsr_01_triggerBuf[this->sampleOffsetIntoNextAudioBuffer] = 1;

    for (number i = this->sampleOffsetIntoNextAudioBuffer; i < this->vs; i++) {
        this->adsr_01_triggerValueBuf[(Index)i] = v;
    }
}

void floatnum_02_out_set(number v) {
    this->adsr_01_trigger_number_set(v);
}

void floatnum_02_input_bang_bang() {
    this->floatnum_02_out_set(this->floatnum_02_stored);
}

void button_02_output_bang() {
    this->random_02_trigger_bang();
    this->random_01_trigger_bang();
    this->delay_01_input_bang();
    this->floatnum_02_input_bang_bang();
}

void button_02_bangval_bang() {
    ;
    this->button_02_output_bang();
}

void loadbang_03_output_bang() {
    this->button_02_bangval_bang();
}

void numberobj_07_output_set(number v) {
    this->param_04_value_set(v);
}

void numberobj_07_value_set(number v) {
    this->numberobj_07_value_setter(v);
    v = this->numberobj_07_value;
    number localvalue = v;

    if ((bool)(true) && localvalue < 0) {
        localvalue = 0;
    }

    if ((bool)(true) && localvalue > 1) {
        localvalue = 1;
    }

    if (this->numberobj_07_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_07_output_set(localvalue);
}

void expr_05_in2_set(number v) {
    this->expr_05_in2 = v;
}

void random_04_result_set(number v) {
    this->expr_05_in2_set(v);
}

void random_04_trigger_bang() {
    this->random_04_result_set(rnbo_abs(this->random_04_nz_next()) * this->random_04_limit);
}

number delay_02_time_constrain(number v) const {
    if (v < 0)
        v = 0;

    return v;
}

void delay_02_time_set(number v) {
    v = this->delay_02_time_constrain(v);
    this->delay_02_time = v;
}

void expr_05_out1_set(number v) {
    this->expr_05_out1 = v;
    this->delay_02_time_set(this->expr_05_out1);
}

void expr_05_in1_set(number in1) {
    this->expr_05_in1 = in1;
    this->expr_05_out1_set(this->expr_05_in1 + this->expr_05_in2);//#map:+_obj-298:1
}

void random_03_result_set(number v) {
    this->expr_05_in1_set(v);
}

void random_03_trigger_bang() {
    this->random_03_result_set(rnbo_abs(this->random_03_nz_next()) * this->random_03_limit);
}

void delay_02_stop_bang() {
    this->getEngine()->flushClockEvents(this, -1357044121, false);;
}

void delay_02_input_bang() {
    if ((bool)(!(bool)(this->delay_02_delayall)))
        this->delay_02_stop_bang();

    this->getEngine()->scheduleClockEvent(this, -1357044121, this->delay_02_time + this->_currentTime);;
}

void adsr_02_trigger_number_set(number v) {
    this->adsr_02_trigger_number = v;

    if (v != 0)
        this->adsr_02_triggerBuf[this->sampleOffsetIntoNextAudioBuffer] = 1;

    for (number i = this->sampleOffsetIntoNextAudioBuffer; i < this->vs; i++) {
        this->adsr_02_triggerValueBuf[(Index)i] = v;
    }
}

void floatnum_07_out_set(number v) {
    this->adsr_02_trigger_number_set(v);
}

void floatnum_07_input_bang_bang() {
    this->floatnum_07_out_set(this->floatnum_07_stored);
}

void button_03_output_bang() {
    this->random_04_trigger_bang();
    this->random_03_trigger_bang();
    this->delay_02_input_bang();
    this->floatnum_07_input_bang_bang();
}

void button_03_bangval_bang() {
    ;
    this->button_03_output_bang();
}

void loadbang_04_output_bang() {
    this->button_03_bangval_bang();
}

void numberobj_09_output_set(number v) {
    this->param_05_value_set(v);
}

void numberobj_09_value_set(number v) {
    this->numberobj_09_value_setter(v);
    v = this->numberobj_09_value;
    number localvalue = v;

    if ((bool)(true) && localvalue < -60) {
        localvalue = -60;
    }

    if ((bool)(true) && localvalue > 0) {
        localvalue = 0;
    }

    if (this->numberobj_09_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_09_output_set(localvalue);
}

void noise_tilde_01_perform(SampleValue * out, Index n) {
    for (Index i = 0; i < n; i++) {
        out[(Index)i] = this->noise_tilde_01_nz_next();
    }
}

void pink_tilde_01_perform(SampleValue * out1, Index n) {
    auto __pink_tilde_01_numRows = this->pink_tilde_01_numRows;
    auto __pink_tilde_01_runningSum = this->pink_tilde_01_runningSum;
    auto __pink_tilde_01_indexMask = this->pink_tilde_01_indexMask;
    auto __pink_tilde_01_index = this->pink_tilde_01_index;
    number tmp = 0x007fffff;
    Index i;

    for (i = 0; i < (Index)n; i++) {
        Int newRandom;
        Int sum;
        __pink_tilde_01_index = (BinOpInt)((BinOpInt)(__pink_tilde_01_index + 1) & __pink_tilde_01_indexMask);

        if (__pink_tilde_01_index != 0) {
            Int numZeros = 0;
            Int n = (Int)(__pink_tilde_01_index);

            while (((BinOpInt)((BinOpInt)n & (BinOpInt)1)) == 0) {
                n = (BinOpInt)((BinOpInt)n >> imod_nocast((UBinOpInt)1, 32));
                numZeros++;
            }

            __pink_tilde_01_runningSum -= this->pink_tilde_01_rows[(Index)numZeros];
            newRandom = (BinOpInt)((BinOpInt)this->pink_tilde_01_generateRandomNumber() & (BinOpInt)0x007fffff);
            __pink_tilde_01_runningSum += newRandom;
            this->pink_tilde_01_rows[(Index)numZeros] = newRandom;
        }

        newRandom = (BinOpInt)((BinOpInt)this->pink_tilde_01_generateRandomNumber() & (BinOpInt)0x007fffff);
        sum = __pink_tilde_01_runningSum + newRandom;
        out1[(Index)i] = sum / (tmp * __pink_tilde_01_numRows) * 2. - 1.;
    }

    this->pink_tilde_01_index = __pink_tilde_01_index;
    this->pink_tilde_01_runningSum = __pink_tilde_01_runningSum;
}

void dspexpr_04_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
    RNBO_UNUSED(in2);
    Index i;

    for (i = 0; i < (Index)n; i++) {
        out1[(Index)i] = in1[(Index)i] * 4;//#map:_###_obj_###_:1
    }
}

void dspexpr_03_perform(
    const Sample * in1,
    const Sample * in2,
    number in3,
    SampleValue * out1,
    Index n
) {
    Index i;

    for (i = 0; i < (Index)n; i++) {
        out1[(Index)i] = in1[(Index)i] + in3 * (in2[(Index)i] - in1[(Index)i]);//#map:_###_obj_###_:1
    }
}

void dspexpr_02_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
    RNBO_UNUSED(in2);
    Index i;

    for (i = 0; i < (Index)n; i++) {
        out1[(Index)i] = in1[(Index)i] * 0.5;//#map:_###_obj_###_:1
    }
}

void gen_01_perform(const Sample * in1, number in2, number in3, SampleValue * out1, Index n) {
    auto __gen_01_history_1_value = this->gen_01_history_1_value;
    auto __gen_01_history_4_value = this->gen_01_history_4_value;
    auto __gen_01_history_3_value = this->gen_01_history_3_value;
    auto __gen_01_history_2_value = this->gen_01_history_2_value;
    auto min_5_0 = this->maximum(in2, 0);
    auto min_6_1 = this->maximum(in3, 0.01);
    number omega_2 = (this->sr == 0. ? 0. : min_5_0 * 6.28318530717958647692 / this->sr);
    number sn_3 = rnbo_sin(omega_2);
    number cs_4 = rnbo_cos(omega_2);
    number alpha_5 = (min_6_1 == 0. ? 0. : sn_3 * 0.5 / min_6_1);
    number b0_6 = (1 + alpha_5 == 0. ? 0. : (number)1 / (1 + alpha_5));
    number a0_7 = alpha_5 * b0_6;
    number a2_8 = -alpha_5 * b0_6;
    number b1_9 = -2 * cs_4 * b0_6;
    number b2_10 = (1 - alpha_5) * b0_6;
    number expr_7_11 = a0_7;
    number expr_8_12 = 0;
    number expr_9_13 = a2_8;
    number expr_10_14 = b1_9;
    number expr_11_15 = b2_10;
    Index i;

    for (i = 0; i < (Index)n; i++) {
        number mul_12_16 = __gen_01_history_2_value * expr_9_13;
        number mul_13_17 = __gen_01_history_3_value * expr_8_12;
        number mul_14_18 = in1[(Index)i] * expr_7_11;
        number mul_15_19 = __gen_01_history_4_value * expr_10_14;
        number mul_16_20 = __gen_01_history_1_value * expr_11_15;
        number sub_17_21 = mul_14_18 + mul_13_17 + mul_12_16 - (mul_16_20 + mul_15_19);
        number gen_18_22 = sub_17_21;
        number history_1_next_19_23 = fixdenorm(__gen_01_history_4_value);
        number history_2_next_20_24 = fixdenorm(__gen_01_history_3_value);
        number history_3_next_21_25 = fixdenorm(in1[(Index)i]);
        number history_4_next_22_26 = fixdenorm(sub_17_21);
        out1[(Index)i] = gen_18_22;
        __gen_01_history_1_value = history_1_next_19_23;
        __gen_01_history_2_value = history_2_next_20_24;
        __gen_01_history_3_value = history_3_next_21_25;
        __gen_01_history_4_value = history_4_next_22_26;
    }

    this->gen_01_history_2_value = __gen_01_history_2_value;
    this->gen_01_history_3_value = __gen_01_history_3_value;
    this->gen_01_history_4_value = __gen_01_history_4_value;
    this->gen_01_history_1_value = __gen_01_history_1_value;
}

void dspexpr_01_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
    Index i;

    for (i = 0; i < (Index)n; i++) {
        out1[(Index)i] = in1[(Index)i] * in2;//#map:_###_obj_###_:1
    }
}

void adsr_01_perform(
    number attack,
    number decay,
    number sustain,
    number release,
    const SampleValue * trigger_signal,
    SampleValue * out,
    Index n
) {
    RNBO_UNUSED(trigger_signal);
    auto __adsr_01_trigger_number = this->adsr_01_trigger_number;
    auto __adsr_01_time = this->adsr_01_time;
    auto __adsr_01_amplitude = this->adsr_01_amplitude;
    auto __adsr_01_outval = this->adsr_01_outval;
    auto __adsr_01_startingpoint = this->adsr_01_startingpoint;
    auto __adsr_01_phase = this->adsr_01_phase;
    auto __adsr_01_legato = this->adsr_01_legato;
    auto __adsr_01_lastTriggerVal = this->adsr_01_lastTriggerVal;
    auto __adsr_01_maxsustain = this->adsr_01_maxsustain;
    auto __adsr_01_mspersamp = this->adsr_01_mspersamp;
    bool bangMute = false;

    for (Index i = 0; i < n; i++) {
        number clampedattack = (attack > __adsr_01_mspersamp ? attack : __adsr_01_mspersamp);
        number clampeddecay = (decay > __adsr_01_mspersamp ? decay : __adsr_01_mspersamp);
        number clampedsustain = (__adsr_01_maxsustain > __adsr_01_mspersamp ? __adsr_01_maxsustain : __adsr_01_mspersamp);
        number clampedrelease = (release > __adsr_01_mspersamp ? release : __adsr_01_mspersamp);
        number currentTriggerVal = this->adsr_01_triggerValueBuf[(Index)i];

        if ((__adsr_01_lastTriggerVal == 0.0 && currentTriggerVal != 0.0) || this->adsr_01_triggerBuf[(Index)i] == 1) {
            if ((bool)(__adsr_01_legato)) {
                if (__adsr_01_phase != 0) {
                    __adsr_01_startingpoint = __adsr_01_outval;
                } else {
                    __adsr_01_startingpoint = 0;
                }
            } else {
                __adsr_01_startingpoint = 0;
            }

            __adsr_01_amplitude = currentTriggerVal;
            __adsr_01_phase = 3;
            __adsr_01_time = 0.0;
            bangMute = false;
        } else if (__adsr_01_lastTriggerVal != 0.0 && currentTriggerVal == 0.0) {
            if (__adsr_01_phase != 4 && __adsr_01_phase != 0) {
                __adsr_01_phase = 4;
                __adsr_01_amplitude = __adsr_01_outval;
                __adsr_01_time = 0.0;
            }
        }

        __adsr_01_time += __adsr_01_mspersamp;

        if (__adsr_01_phase == 0) {
            __adsr_01_outval = 0;
        } else if (__adsr_01_phase == 3) {
            if (__adsr_01_time > clampedattack) {
                __adsr_01_time -= clampedattack;
                __adsr_01_phase = 2;
                __adsr_01_outval = __adsr_01_amplitude;
            } else {
                __adsr_01_outval = (__adsr_01_amplitude - __adsr_01_startingpoint) * __adsr_01_time / clampedattack + __adsr_01_startingpoint;
            }
        } else if (__adsr_01_phase == 2) {
            if (__adsr_01_time > clampeddecay) {
                __adsr_01_time -= clampeddecay;
                __adsr_01_phase = 1;
                __adsr_01_outval = __adsr_01_amplitude * sustain;
            } else {
                __adsr_01_outval = __adsr_01_amplitude * sustain + (__adsr_01_amplitude - __adsr_01_amplitude * sustain) * (1. - __adsr_01_time / clampeddecay);
            }
        } else if (__adsr_01_phase == 1) {
            if (__adsr_01_time > clampedsustain && __adsr_01_maxsustain > -1) {
                __adsr_01_time -= clampedsustain;
                __adsr_01_phase = 4;
                __adsr_01_amplitude = __adsr_01_outval;
            } else {
                __adsr_01_outval = __adsr_01_amplitude * sustain;
            }
        } else if (__adsr_01_phase == 4) {
            if (__adsr_01_time > clampedrelease) {
                __adsr_01_time = 0;
                __adsr_01_phase = 0;
                __adsr_01_outval = 0;
                __adsr_01_amplitude = 0;
                bangMute = true;
            } else {
                __adsr_01_outval = __adsr_01_amplitude * (1.0 - __adsr_01_time / clampedrelease);
            }
        }

        out[(Index)i] = __adsr_01_outval;
        this->adsr_01_triggerBuf[(Index)i] = 0;
        this->adsr_01_triggerValueBuf[(Index)i] = __adsr_01_trigger_number;
        __adsr_01_lastTriggerVal = currentTriggerVal;
    }

    if ((bool)(bangMute)) {
        this->getEngine()->scheduleClockEventWithValue(
            this,
            -1096817259,
            this->sampsToMs((SampleIndex)(this->vs)) + this->_currentTime,
            0
        );;
    }

    this->adsr_01_lastTriggerVal = __adsr_01_lastTriggerVal;
    this->adsr_01_phase = __adsr_01_phase;
    this->adsr_01_startingpoint = __adsr_01_startingpoint;
    this->adsr_01_outval = __adsr_01_outval;
    this->adsr_01_amplitude = __adsr_01_amplitude;
    this->adsr_01_time = __adsr_01_time;
}

void noise_tilde_02_perform(SampleValue * out, Index n) {
    for (Index i = 0; i < n; i++) {
        out[(Index)i] = this->noise_tilde_02_nz_next();
    }
}

void dspexpr_06_perform(const Sample * in1, const Sample * in2, SampleValue * out1, Index n) {
    Index i;

    for (i = 0; i < (Index)n; i++) {
        out1[(Index)i] = in1[(Index)i] * in2[(Index)i];//#map:_###_obj_###_:1
    }
}

void dspexpr_05_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
    Index i;

    for (i = 0; i < (Index)n; i++) {
        out1[(Index)i] = in1[(Index)i] * in2;//#map:_###_obj_###_:1
    }
}

void signaladder_01_perform(
    const SampleValue * in1,
    const SampleValue * in2,
    SampleValue * out,
    Index n
) {
    Index i;

    for (i = 0; i < (Index)n; i++) {
        out[(Index)i] = in1[(Index)i] + in2[(Index)i];
    }
}

void adsr_02_perform(
    number attack,
    number decay,
    number sustain,
    number release,
    const SampleValue * trigger_signal,
    SampleValue * out,
    Index n
) {
    RNBO_UNUSED(trigger_signal);
    auto __adsr_02_trigger_number = this->adsr_02_trigger_number;
    auto __adsr_02_time = this->adsr_02_time;
    auto __adsr_02_amplitude = this->adsr_02_amplitude;
    auto __adsr_02_outval = this->adsr_02_outval;
    auto __adsr_02_startingpoint = this->adsr_02_startingpoint;
    auto __adsr_02_phase = this->adsr_02_phase;
    auto __adsr_02_legato = this->adsr_02_legato;
    auto __adsr_02_lastTriggerVal = this->adsr_02_lastTriggerVal;
    auto __adsr_02_maxsustain = this->adsr_02_maxsustain;
    auto __adsr_02_mspersamp = this->adsr_02_mspersamp;
    bool bangMute = false;

    for (Index i = 0; i < n; i++) {
        number clampedattack = (attack > __adsr_02_mspersamp ? attack : __adsr_02_mspersamp);
        number clampeddecay = (decay > __adsr_02_mspersamp ? decay : __adsr_02_mspersamp);
        number clampedsustain = (__adsr_02_maxsustain > __adsr_02_mspersamp ? __adsr_02_maxsustain : __adsr_02_mspersamp);
        number clampedrelease = (release > __adsr_02_mspersamp ? release : __adsr_02_mspersamp);
        number currentTriggerVal = this->adsr_02_triggerValueBuf[(Index)i];

        if ((__adsr_02_lastTriggerVal == 0.0 && currentTriggerVal != 0.0) || this->adsr_02_triggerBuf[(Index)i] == 1) {
            if ((bool)(__adsr_02_legato)) {
                if (__adsr_02_phase != 0) {
                    __adsr_02_startingpoint = __adsr_02_outval;
                } else {
                    __adsr_02_startingpoint = 0;
                }
            } else {
                __adsr_02_startingpoint = 0;
            }

            __adsr_02_amplitude = currentTriggerVal;
            __adsr_02_phase = 3;
            __adsr_02_time = 0.0;
            bangMute = false;
        } else if (__adsr_02_lastTriggerVal != 0.0 && currentTriggerVal == 0.0) {
            if (__adsr_02_phase != 4 && __adsr_02_phase != 0) {
                __adsr_02_phase = 4;
                __adsr_02_amplitude = __adsr_02_outval;
                __adsr_02_time = 0.0;
            }
        }

        __adsr_02_time += __adsr_02_mspersamp;

        if (__adsr_02_phase == 0) {
            __adsr_02_outval = 0;
        } else if (__adsr_02_phase == 3) {
            if (__adsr_02_time > clampedattack) {
                __adsr_02_time -= clampedattack;
                __adsr_02_phase = 2;
                __adsr_02_outval = __adsr_02_amplitude;
            } else {
                __adsr_02_outval = (__adsr_02_amplitude - __adsr_02_startingpoint) * __adsr_02_time / clampedattack + __adsr_02_startingpoint;
            }
        } else if (__adsr_02_phase == 2) {
            if (__adsr_02_time > clampeddecay) {
                __adsr_02_time -= clampeddecay;
                __adsr_02_phase = 1;
                __adsr_02_outval = __adsr_02_amplitude * sustain;
            } else {
                __adsr_02_outval = __adsr_02_amplitude * sustain + (__adsr_02_amplitude - __adsr_02_amplitude * sustain) * (1. - __adsr_02_time / clampeddecay);
            }
        } else if (__adsr_02_phase == 1) {
            if (__adsr_02_time > clampedsustain && __adsr_02_maxsustain > -1) {
                __adsr_02_time -= clampedsustain;
                __adsr_02_phase = 4;
                __adsr_02_amplitude = __adsr_02_outval;
            } else {
                __adsr_02_outval = __adsr_02_amplitude * sustain;
            }
        } else if (__adsr_02_phase == 4) {
            if (__adsr_02_time > clampedrelease) {
                __adsr_02_time = 0;
                __adsr_02_phase = 0;
                __adsr_02_outval = 0;
                __adsr_02_amplitude = 0;
                bangMute = true;
            } else {
                __adsr_02_outval = __adsr_02_amplitude * (1.0 - __adsr_02_time / clampedrelease);
            }
        }

        out[(Index)i] = __adsr_02_outval;
        this->adsr_02_triggerBuf[(Index)i] = 0;
        this->adsr_02_triggerValueBuf[(Index)i] = __adsr_02_trigger_number;
        __adsr_02_lastTriggerVal = currentTriggerVal;
    }

    if ((bool)(bangMute)) {
        this->getEngine()->scheduleClockEventWithValue(
            this,
            -1468824490,
            this->sampsToMs((SampleIndex)(this->vs)) + this->_currentTime,
            0
        );;
    }

    this->adsr_02_lastTriggerVal = __adsr_02_lastTriggerVal;
    this->adsr_02_phase = __adsr_02_phase;
    this->adsr_02_startingpoint = __adsr_02_startingpoint;
    this->adsr_02_outval = __adsr_02_outval;
    this->adsr_02_amplitude = __adsr_02_amplitude;
    this->adsr_02_time = __adsr_02_time;
}

void noise_tilde_03_perform(SampleValue * out, Index n) {
    for (Index i = 0; i < n; i++) {
        out[(Index)i] = this->noise_tilde_03_nz_next();
    }
}

void dspexpr_08_perform(const Sample * in1, const Sample * in2, SampleValue * out1, Index n) {
    Index i;

    for (i = 0; i < (Index)n; i++) {
        out1[(Index)i] = in1[(Index)i] * in2[(Index)i];//#map:_###_obj_###_:1
    }
}

void dspexpr_07_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
    Index i;

    for (i = 0; i < (Index)n; i++) {
        out1[(Index)i] = in1[(Index)i] * in2;//#map:_###_obj_###_:1
    }
}

void signaladder_02_perform(
    const SampleValue * in1,
    const SampleValue * in2,
    SampleValue * out,
    Index n
) {
    Index i;

    for (i = 0; i < (Index)n; i++) {
        out[(Index)i] = in1[(Index)i] + in2[(Index)i];
    }
}

void stackprotect_perform(Index n) {
    RNBO_UNUSED(n);
    auto __stackprotect_count = this->stackprotect_count;
    __stackprotect_count = 0;
    this->stackprotect_count = __stackprotect_count;
}

void numberobj_01_value_setter(number v) {
    number localvalue = v;

    if ((bool)(true) && localvalue < 0) {
        localvalue = 0;
    }

    if ((bool)(true) && localvalue > 1) {
        localvalue = 1;
    }

    if (this->numberobj_01_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_01_value = localvalue;
}

void numberobj_02_value_setter(number v) {
    number localvalue = v;

    if ((bool)(true) && localvalue < 36) {
        localvalue = 36;
    }

    if ((bool)(true) && localvalue > 127) {
        localvalue = 127;
    }

    if (this->numberobj_02_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_02_value = localvalue;
}

void numberobj_03_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_03_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_03_value = localvalue;
}

void numberobj_04_value_setter(number v) {
    number localvalue = v;

    if ((bool)(true) && localvalue < 0) {
        localvalue = 0;
    }

    if ((bool)(true) && localvalue > 1) {
        localvalue = 1;
    }

    if (this->numberobj_04_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_04_value = localvalue;
}

void numberobj_05_value_setter(number v) {
    number localvalue = v;

    if ((bool)(true) && localvalue < -60) {
        localvalue = -60;
    }

    if ((bool)(true) && localvalue > 0) {
        localvalue = 0;
    }

    if (this->numberobj_05_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_05_value = localvalue;
}

void numberobj_06_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_06_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_06_value = localvalue;
}

void random_01_limit_setter(number v) {
    if (v >= 0)
        this->random_01_limit = v;
}

void random_02_limit_setter(number v) {
    if (v >= 0)
        this->random_02_limit = v;
}

void numberobj_07_value_setter(number v) {
    number localvalue = v;

    if ((bool)(true) && localvalue < 0) {
        localvalue = 0;
    }

    if ((bool)(true) && localvalue > 1) {
        localvalue = 1;
    }

    if (this->numberobj_07_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_07_value = localvalue;
}

void numberobj_08_value_setter(number v) {
    number localvalue = v;

    if (this->numberobj_08_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_08_value = localvalue;
}

void random_03_limit_setter(number v) {
    if (v >= 0)
        this->random_03_limit = v;
}

void random_04_limit_setter(number v) {
    if (v >= 0)
        this->random_04_limit = v;
}

void numberobj_09_value_setter(number v) {
    number localvalue = v;

    if ((bool)(true) && localvalue < -60) {
        localvalue = -60;
    }

    if ((bool)(true) && localvalue > 0) {
        localvalue = 0;
    }

    if (this->numberobj_09_currentFormat != 6) {
        localvalue = trunc(localvalue);
    }

    this->numberobj_09_value = localvalue;
}

void numberobj_01_init() {
    this->numberobj_01_currentFormat = 6;
}

void numberobj_01_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_01_value;
}

void numberobj_01_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_01_value_set(preset["value"]);
}

void param_01_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_01_value;
}

void param_01_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_01_value_set(preset["value"]);
}

void noise_tilde_01_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->noise_tilde_01_nz_state
    );
}

void noise_tilde_01_nz_init() {
    this->noise_tilde_01_nz_reset();
}

void noise_tilde_01_nz_seed(number v) {
    xoshiro_reset(v, this->noise_tilde_01_nz_state);
}

number noise_tilde_01_nz_next() {
    return xoshiro_next(this->noise_tilde_01_nz_state);
}

void noise_tilde_01_dspsetup(bool force) {
    if ((bool)(this->noise_tilde_01_setupDone) && (bool)(!(bool)(force)))
        return;

    this->noise_tilde_01_setupDone = true;
}

number gen_01_history_1_getvalue() {
    return this->gen_01_history_1_value;
}

void gen_01_history_1_setvalue(number val) {
    this->gen_01_history_1_value = val;
}

void gen_01_history_1_reset() {
    this->gen_01_history_1_value = 0;
}

void gen_01_history_1_init() {
    this->gen_01_history_1_value = 0;
}

number gen_01_history_2_getvalue() {
    return this->gen_01_history_2_value;
}

void gen_01_history_2_setvalue(number val) {
    this->gen_01_history_2_value = val;
}

void gen_01_history_2_reset() {
    this->gen_01_history_2_value = 0;
}

void gen_01_history_2_init() {
    this->gen_01_history_2_value = 0;
}

number gen_01_history_3_getvalue() {
    return this->gen_01_history_3_value;
}

void gen_01_history_3_setvalue(number val) {
    this->gen_01_history_3_value = val;
}

void gen_01_history_3_reset() {
    this->gen_01_history_3_value = 0;
}

void gen_01_history_3_init() {
    this->gen_01_history_3_value = 0;
}

number gen_01_history_4_getvalue() {
    return this->gen_01_history_4_value;
}

void gen_01_history_4_setvalue(number val) {
    this->gen_01_history_4_value = val;
}

void gen_01_history_4_reset() {
    this->gen_01_history_4_value = 0;
}

void gen_01_history_4_init() {
    this->gen_01_history_4_value = 0;
}

void param_02_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_02_value;
}

void param_02_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_02_value_set(preset["value"]);
}

void numberobj_02_init() {
    this->numberobj_02_currentFormat = 6;
}

void numberobj_02_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_02_value;
}

void numberobj_02_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_02_value_set(preset["value"]);
}

void pink_tilde_01_init() {
    this->pink_tilde_01_last = systemticks();
    this->pink_tilde_01_index = 0;
    this->pink_tilde_01_indexMask = ((BinOpInt)((BinOpInt)1 << imod_nocast((UBinOpInt)this->pink_tilde_01_numRows, 32))) - 1;
    number tmp = this->pink_tilde_01_randomShift - 1;
    this->pink_tilde_01_scalar = (number)1 / ((this->pink_tilde_01_numRows + 1) * ((BinOpInt)((BinOpInt)1 << imod_nocast((UBinOpInt)tmp, 32))));
    this->pink_tilde_01_runningSum = 0;
}

Index pink_tilde_01_generateRandomNumber() {
    this->pink_tilde_01_last = (BinOpInt)((BinOpInt)(rnbo_imul(this->pink_tilde_01_last, 196314165) + 907633515) | (BinOpInt)0);
    Int tmp = (Int)(this->pink_tilde_01_last);
    return tmp;
}

void numberobj_03_init() {
    this->numberobj_03_currentFormat = 6;
}

void numberobj_03_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_03_value;
}

void numberobj_03_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_03_value_set(preset["value"]);
}

void numberobj_04_init() {
    this->numberobj_04_currentFormat = 6;
}

void numberobj_04_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_04_value;
}

void numberobj_04_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_04_value_set(preset["value"]);
}

void numberobj_05_init() {
    this->numberobj_05_currentFormat = 6;
}

void numberobj_05_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_05_value;
}

void numberobj_05_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_05_value_set(preset["value"]);
}

void param_03_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_03_value;
}

void param_03_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_03_value_set(preset["value"]);
}

void adsr_01_dspsetup(bool force) {
    if ((bool)(this->adsr_01_setupDone) && (bool)(!(bool)(force)))
        return;

    this->adsr_01_mspersamp = (number)1000 / this->sr;
    this->adsr_01_setupDone = true;
}

void noise_tilde_02_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->noise_tilde_02_nz_state
    );
}

void noise_tilde_02_nz_init() {
    this->noise_tilde_02_nz_reset();
}

void noise_tilde_02_nz_seed(number v) {
    xoshiro_reset(v, this->noise_tilde_02_nz_state);
}

number noise_tilde_02_nz_next() {
    return xoshiro_next(this->noise_tilde_02_nz_state);
}

void noise_tilde_02_dspsetup(bool force) {
    if ((bool)(this->noise_tilde_02_setupDone) && (bool)(!(bool)(force)))
        return;

    this->noise_tilde_02_setupDone = true;
}

void numberobj_06_init() {
    this->numberobj_06_currentFormat = 6;
}

void numberobj_06_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_06_value;
}

void numberobj_06_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_06_value_set(preset["value"]);
}

void random_01_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_01_nz_state
    );
}

void random_01_nz_init() {
    this->random_01_nz_reset();
}

void random_01_nz_seed(number v) {
    xoshiro_reset(v, this->random_01_nz_state);
}

number random_01_nz_next() {
    return xoshiro_next(this->random_01_nz_state);
}

void random_02_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_02_nz_state
    );
}

void random_02_nz_init() {
    this->random_02_nz_reset();
}

void random_02_nz_seed(number v) {
    xoshiro_reset(v, this->random_02_nz_state);
}

number random_02_nz_next() {
    return xoshiro_next(this->random_02_nz_state);
}

void param_04_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_04_value;
}

void param_04_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_04_value_set(preset["value"]);
}

void numberobj_07_init() {
    this->numberobj_07_currentFormat = 6;
}

void numberobj_07_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_07_value;
}

void numberobj_07_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_07_value_set(preset["value"]);
}

void adsr_02_dspsetup(bool force) {
    if ((bool)(this->adsr_02_setupDone) && (bool)(!(bool)(force)))
        return;

    this->adsr_02_mspersamp = (number)1000 / this->sr;
    this->adsr_02_setupDone = true;
}

void noise_tilde_03_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->noise_tilde_03_nz_state
    );
}

void noise_tilde_03_nz_init() {
    this->noise_tilde_03_nz_reset();
}

void noise_tilde_03_nz_seed(number v) {
    xoshiro_reset(v, this->noise_tilde_03_nz_state);
}

number noise_tilde_03_nz_next() {
    return xoshiro_next(this->noise_tilde_03_nz_state);
}

void noise_tilde_03_dspsetup(bool force) {
    if ((bool)(this->noise_tilde_03_setupDone) && (bool)(!(bool)(force)))
        return;

    this->noise_tilde_03_setupDone = true;
}

void numberobj_08_init() {
    this->numberobj_08_currentFormat = 6;
}

void numberobj_08_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_08_value;
}

void numberobj_08_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_08_value_set(preset["value"]);
}

void random_03_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_03_nz_state
    );
}

void random_03_nz_init() {
    this->random_03_nz_reset();
}

void random_03_nz_seed(number v) {
    xoshiro_reset(v, this->random_03_nz_state);
}

number random_03_nz_next() {
    return xoshiro_next(this->random_03_nz_state);
}

void random_04_nz_reset() {
    xoshiro_reset(
        systemticks() + this->voice() + this->random(0, 10000),
        this->random_04_nz_state
    );
}

void random_04_nz_init() {
    this->random_04_nz_reset();
}

void random_04_nz_seed(number v) {
    xoshiro_reset(v, this->random_04_nz_state);
}

number random_04_nz_next() {
    return xoshiro_next(this->random_04_nz_state);
}

void param_05_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_05_value;
}

void param_05_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_05_value_set(preset["value"]);
}

void numberobj_09_init() {
    this->numberobj_09_currentFormat = 6;
}

void numberobj_09_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->numberobj_09_value;
}

void numberobj_09_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->numberobj_09_value_set(preset["value"]);
}

void globaltransport_advance() {}

void globaltransport_dspsetup(bool ) {}

bool stackprotect_check() {
    this->stackprotect_count++;

    if (this->stackprotect_count > 128) {
        console->log("STACK OVERFLOW DETECTED - stopped processing branch !");
        return true;
    }

    return false;
}

Index getPatcherSerial() const {
    return 0;
}

void sendParameter(ParameterIndex index, bool ignoreValue) {
    this->getEngine()->notifyParameterValueChanged(index, (ignoreValue ? 0 : this->getParameterValue(index)), ignoreValue);
}

void scheduleParamInit(ParameterIndex index, Index order) {
    this->paramInitIndices->push(index);
    this->paramInitOrder->push(order);
}

void processParamInitEvents() {
    this->listquicksort(
        this->paramInitOrder,
        this->paramInitIndices,
        0,
        (int)(this->paramInitOrder->length - 1),
        true
    );

    for (Index i = 0; i < this->paramInitOrder->length; i++) {
        this->getEngine()->scheduleParameterBang(this->paramInitIndices[i], 0);
    }
}

void updateTime(MillisecondTime time, EXTERNALENGINE* engine, bool inProcess = false) {
    RNBO_UNUSED(inProcess);
    RNBO_UNUSED(engine);
    this->_currentTime = time;
    auto offset = rnbo_fround(this->msToSamps(time - this->getEngine()->getCurrentTime(), this->sr));

    if (offset >= (SampleIndex)(this->vs))
        offset = (SampleIndex)(this->vs) - 1;

    if (offset < 0)
        offset = 0;

    this->sampleOffsetIntoNextAudioBuffer = (Index)(offset);
}

void assign_defaults()
{
    numberobj_01_value = 0;
    numberobj_01_value_setter(numberobj_01_value);
    send_01_input_number = 0;
    param_01_value = 0;
    noise_tilde_01_seed = 0;
    dspexpr_01_in1 = 0;
    dspexpr_01_in2 = 0;
    gen_01_in1 = 0;
    gen_01_in2 = 0;
    gen_01_in3 = 0;
    dspexpr_02_in1 = 0;
    dspexpr_02_in2 = 0.5;
    dspexpr_03_in1 = 0;
    dspexpr_03_in2 = 0;
    dspexpr_03_in3 = 0;
    send_02_input_number = 0;
    param_02_value = 64;
    numberobj_02_value = 0;
    numberobj_02_value_setter(numberobj_02_value);
    dspexpr_04_in1 = 0;
    dspexpr_04_in2 = 4;
    floatnum_01_input_number = 0;
    floatnum_01_value = 0;
    numberobj_03_value = 0;
    numberobj_03_value_setter(numberobj_03_value);
    expr_01_in1 = 0;
    expr_01_out1 = 0;
    receive_01_output_number = 0;
    numberobj_04_value = 0;
    numberobj_04_value_setter(numberobj_04_value);
    receive_02_output_number = 0;
    scale_01_inlow = 36;
    scale_01_inhigh = 127;
    scale_01_outlow = 0.9;
    scale_01_outhigh = 0.1;
    scale_01_power = 1;
    receive_03_output_number = 0;
    send_03_input_number = 0;
    numberobj_05_value = 0;
    numberobj_05_value_setter(numberobj_05_value);
    param_03_value = -10;
    dspexpr_05_in1 = 0;
    dspexpr_05_in2 = 0;
    dspexpr_06_in1 = 0;
    dspexpr_06_in2 = 0;
    floatnum_02_input_number = 0;
    floatnum_02_value = 0;
    adsr_01_trigger_number = 0;
    adsr_01_attack = 0;
    adsr_01_decay = 0;
    adsr_01_sustain = 0;
    adsr_01_release = 0;
    adsr_01_legato = false;
    adsr_01_maxsustain = -1;
    noise_tilde_02_seed = 0;
    floatnum_03_input_number = 0;
    floatnum_03_value = 0;
    delay_01_time = 500;
    delay_01_delayall = 1;
    floatnum_04_input_number = 0;
    floatnum_04_value = 0;
    numberobj_06_value = 0;
    numberobj_06_value_setter(numberobj_06_value);
    expr_02_in1 = 0;
    expr_02_out1 = 0;
    receive_04_output_number = 0;
    expr_03_in1 = 0;
    expr_03_in2 = 0;
    expr_03_out1 = 0;
    random_01_limit = 1;
    random_01_limit_setter(random_01_limit);
    random_01_seed = 0;
    random_02_limit = 500;
    random_02_limit_setter(random_02_limit);
    random_02_seed = 1234;
    floatnum_05_input_number = 0;
    floatnum_05_value = 0;
    floatnum_06_input_number = 0;
    floatnum_06_value = 0;
    receive_05_output_number = 0;
    scale_02_inlow = 0;
    scale_02_inhigh = 1;
    scale_02_outlow = 10000;
    scale_02_outhigh = 0;
    scale_02_power = 1;
    send_04_input_number = 0;
    param_04_value = 0.5;
    numberobj_07_value = 0;
    numberobj_07_value_setter(numberobj_07_value);
    dspexpr_07_in1 = 0;
    dspexpr_07_in2 = 0;
    dspexpr_08_in1 = 0;
    dspexpr_08_in2 = 0;
    floatnum_07_input_number = 0;
    floatnum_07_value = 0;
    adsr_02_trigger_number = 0;
    adsr_02_attack = 0;
    adsr_02_decay = 0;
    adsr_02_sustain = 0;
    adsr_02_release = 0;
    adsr_02_legato = false;
    adsr_02_maxsustain = -1;
    noise_tilde_03_seed = 0;
    floatnum_08_input_number = 0;
    floatnum_08_value = 0;
    delay_02_time = 500;
    delay_02_delayall = 1;
    floatnum_09_input_number = 0;
    floatnum_09_value = 0;
    numberobj_08_value = 0;
    numberobj_08_value_setter(numberobj_08_value);
    expr_04_in1 = 0;
    expr_04_out1 = 0;
    receive_06_output_number = 0;
    random_03_limit = 1;
    random_03_limit_setter(random_03_limit);
    random_03_seed = 0;
    expr_05_in1 = 0;
    expr_05_in2 = 0;
    expr_05_out1 = 0;
    random_04_limit = 500;
    random_04_limit_setter(random_04_limit);
    random_04_seed = 2;
    floatnum_10_input_number = 0;
    floatnum_10_value = 0;
    floatnum_11_input_number = 0;
    floatnum_11_value = 0;
    receive_07_output_number = 0;
    scale_03_inlow = 0;
    scale_03_inhigh = 1;
    scale_03_outlow = 10000;
    scale_03_outhigh = 0;
    scale_03_power = 1;
    send_05_input_number = 0;
    param_05_value = -10;
    numberobj_09_value = 0;
    numberobj_09_value_setter(numberobj_09_value);
    _currentTime = 0;
    audioProcessSampleCount = 0;
    sampleOffsetIntoNextAudioBuffer = 0;
    zeroBuffer = nullptr;
    dummyBuffer = nullptr;
    signals[0] = nullptr;
    signals[1] = nullptr;
    signals[2] = nullptr;
    signals[3] = nullptr;
    didAllocateSignals = 0;
    vs = 0;
    maxvs = 0;
    sr = 44100;
    invsr = 0.000022675736961451248;
    numberobj_01_currentFormat = 6;
    numberobj_01_lastValue = 0;
    param_01_lastValue = 0;
    noise_tilde_01_setupDone = false;
    gen_01_history_1_value = 0;
    gen_01_history_2_value = 0;
    gen_01_history_3_value = 0;
    gen_01_history_4_value = 0;
    param_02_lastValue = 0;
    numberobj_02_currentFormat = 6;
    numberobj_02_lastValue = 0;
    floatnum_01_stored = 500;
    pink_tilde_01_randomShift = 8;
    pink_tilde_01_numRows = 16;
    numberobj_03_currentFormat = 6;
    numberobj_03_lastValue = 0;
    numberobj_04_currentFormat = 6;
    numberobj_04_lastValue = 0;
    numberobj_05_currentFormat = 6;
    numberobj_05_lastValue = 0;
    param_03_lastValue = 0;
    floatnum_02_stored = 1;
    adsr_01_phase = 1;
    adsr_01_mspersamp = 0;
    adsr_01_time = 0;
    adsr_01_lastTriggerVal = 0;
    adsr_01_amplitude = 0;
    adsr_01_outval = 0;
    adsr_01_startingpoint = 0;
    adsr_01_triggerBuf = nullptr;
    adsr_01_triggerValueBuf = nullptr;
    adsr_01_setupDone = false;
    noise_tilde_02_setupDone = false;
    floatnum_03_stored = 10;
    floatnum_04_stored = 100;
    numberobj_06_currentFormat = 6;
    numberobj_06_lastValue = 0;
    floatnum_05_stored = 0;
    floatnum_06_stored = 20;
    param_04_lastValue = 0;
    numberobj_07_currentFormat = 6;
    numberobj_07_lastValue = 0;
    floatnum_07_stored = 1;
    adsr_02_phase = 1;
    adsr_02_mspersamp = 0;
    adsr_02_time = 0;
    adsr_02_lastTriggerVal = 0;
    adsr_02_amplitude = 0;
    adsr_02_outval = 0;
    adsr_02_startingpoint = 0;
    adsr_02_triggerBuf = nullptr;
    adsr_02_triggerValueBuf = nullptr;
    adsr_02_setupDone = false;
    noise_tilde_03_setupDone = false;
    floatnum_08_stored = 10;
    floatnum_09_stored = 100;
    numberobj_08_currentFormat = 6;
    numberobj_08_lastValue = 0;
    floatnum_10_stored = 0;
    floatnum_11_stored = 20;
    param_05_lastValue = 0;
    numberobj_09_currentFormat = 6;
    numberobj_09_lastValue = 0;
    globaltransport_tempo = nullptr;
    globaltransport_state = nullptr;
    stackprotect_count = 0;
    _voiceIndex = 0;
    _noteNumber = 0;
    isMuted = 1;
}

    // data ref strings
    struct DataRefStrings {
    	static constexpr auto& name0 = "pink_tilde_01_rowsobj";
    	static constexpr auto& file0 = "";
    	static constexpr auto& tag0 = "buffer~";
    	DataRefStrings* operator->() { return this; }
    	const DataRefStrings* operator->() const { return this; }
    };

    DataRefStrings dataRefStrings;

// member variables

    number numberobj_01_value;
    number send_01_input_number;
    list send_01_input_list;
    number param_01_value;
    number noise_tilde_01_seed;
    number dspexpr_01_in1;
    number dspexpr_01_in2;
    number gen_01_in1;
    number gen_01_in2;
    number gen_01_in3;
    number dspexpr_02_in1;
    number dspexpr_02_in2;
    number dspexpr_03_in1;
    number dspexpr_03_in2;
    number dspexpr_03_in3;
    number send_02_input_number;
    list send_02_input_list;
    number param_02_value;
    number numberobj_02_value;
    number dspexpr_04_in1;
    number dspexpr_04_in2;
    number floatnum_01_input_number;
    number floatnum_01_value;
    number numberobj_03_value;
    number expr_01_in1;
    number expr_01_out1;
    number receive_01_output_number;
    list receive_01_output_list;
    number numberobj_04_value;
    number receive_02_output_number;
    list receive_02_output_list;
    list scale_01_input;
    number scale_01_inlow;
    number scale_01_inhigh;
    number scale_01_outlow;
    number scale_01_outhigh;
    number scale_01_power;
    list scale_01_out;
    number receive_03_output_number;
    list receive_03_output_list;
    number send_03_input_number;
    list send_03_input_list;
    number numberobj_05_value;
    number param_03_value;
    number dspexpr_05_in1;
    number dspexpr_05_in2;
    number dspexpr_06_in1;
    number dspexpr_06_in2;
    number floatnum_02_input_number;
    number floatnum_02_value;
    number adsr_01_trigger_number;
    number adsr_01_attack;
    number adsr_01_decay;
    number adsr_01_sustain;
    number adsr_01_release;
    number adsr_01_legato;
    number adsr_01_maxsustain;
    number noise_tilde_02_seed;
    number floatnum_03_input_number;
    number floatnum_03_value;
    number delay_01_time;
    number delay_01_delayall;
    number floatnum_04_input_number;
    number floatnum_04_value;
    number numberobj_06_value;
    number expr_02_in1;
    number expr_02_out1;
    number receive_04_output_number;
    list receive_04_output_list;
    number expr_03_in1;
    number expr_03_in2;
    number expr_03_out1;
    number random_01_limit;
    number random_01_seed;
    number random_02_limit;
    number random_02_seed;
    number floatnum_05_input_number;
    number floatnum_05_value;
    number floatnum_06_input_number;
    number floatnum_06_value;
    number receive_05_output_number;
    list receive_05_output_list;
    list scale_02_input;
    number scale_02_inlow;
    number scale_02_inhigh;
    number scale_02_outlow;
    number scale_02_outhigh;
    number scale_02_power;
    list scale_02_out;
    number send_04_input_number;
    list send_04_input_list;
    number param_04_value;
    number numberobj_07_value;
    number dspexpr_07_in1;
    number dspexpr_07_in2;
    number dspexpr_08_in1;
    number dspexpr_08_in2;
    number floatnum_07_input_number;
    number floatnum_07_value;
    number adsr_02_trigger_number;
    number adsr_02_attack;
    number adsr_02_decay;
    number adsr_02_sustain;
    number adsr_02_release;
    number adsr_02_legato;
    number adsr_02_maxsustain;
    number noise_tilde_03_seed;
    number floatnum_08_input_number;
    number floatnum_08_value;
    number delay_02_time;
    number delay_02_delayall;
    number floatnum_09_input_number;
    number floatnum_09_value;
    number numberobj_08_value;
    number expr_04_in1;
    number expr_04_out1;
    number receive_06_output_number;
    list receive_06_output_list;
    number random_03_limit;
    number random_03_seed;
    number expr_05_in1;
    number expr_05_in2;
    number expr_05_out1;
    number random_04_limit;
    number random_04_seed;
    number floatnum_10_input_number;
    number floatnum_10_value;
    number floatnum_11_input_number;
    number floatnum_11_value;
    number receive_07_output_number;
    list receive_07_output_list;
    list scale_03_input;
    number scale_03_inlow;
    number scale_03_inhigh;
    number scale_03_outlow;
    number scale_03_outhigh;
    number scale_03_power;
    list scale_03_out;
    number send_05_input_number;
    list send_05_input_list;
    number param_05_value;
    number numberobj_09_value;
    MillisecondTime _currentTime;
    ENGINE _internalEngine;
    UInt64 audioProcessSampleCount;
    Index sampleOffsetIntoNextAudioBuffer;
    signal zeroBuffer;
    signal dummyBuffer;
    SampleValue * signals[4];
    bool didAllocateSignals;
    Index vs;
    Index maxvs;
    number sr;
    number invsr;
    Int numberobj_01_currentFormat;
    number numberobj_01_lastValue;
    number param_01_lastValue;
    UInt noise_tilde_01_nz_state[4] = { };
    bool noise_tilde_01_setupDone;
    number gen_01_history_1_value;
    number gen_01_history_2_value;
    number gen_01_history_3_value;
    number gen_01_history_4_value;
    number param_02_lastValue;
    Int numberobj_02_currentFormat;
    number numberobj_02_lastValue;
    number floatnum_01_stored;
    Index pink_tilde_01_last;
    Int pink_tilde_01_randomShift;
    number pink_tilde_01_numRows;
    IntBufferRef pink_tilde_01_rows;
    Int pink_tilde_01_runningSum;
    Int pink_tilde_01_index;
    Int pink_tilde_01_indexMask;
    number pink_tilde_01_scalar;
    Int numberobj_03_currentFormat;
    number numberobj_03_lastValue;
    Int numberobj_04_currentFormat;
    number numberobj_04_lastValue;
    Int numberobj_05_currentFormat;
    number numberobj_05_lastValue;
    number param_03_lastValue;
    number floatnum_02_stored;
    Int adsr_01_phase;
    number adsr_01_mspersamp;
    number adsr_01_time;
    number adsr_01_lastTriggerVal;
    number adsr_01_amplitude;
    number adsr_01_outval;
    number adsr_01_startingpoint;
    signal adsr_01_triggerBuf;
    signal adsr_01_triggerValueBuf;
    bool adsr_01_setupDone;
    UInt noise_tilde_02_nz_state[4] = { };
    bool noise_tilde_02_setupDone;
    number floatnum_03_stored;
    number floatnum_04_stored;
    Int numberobj_06_currentFormat;
    number numberobj_06_lastValue;
    UInt random_01_nz_state[4] = { };
    UInt random_02_nz_state[4] = { };
    number floatnum_05_stored;
    number floatnum_06_stored;
    number param_04_lastValue;
    Int numberobj_07_currentFormat;
    number numberobj_07_lastValue;
    number floatnum_07_stored;
    Int adsr_02_phase;
    number adsr_02_mspersamp;
    number adsr_02_time;
    number adsr_02_lastTriggerVal;
    number adsr_02_amplitude;
    number adsr_02_outval;
    number adsr_02_startingpoint;
    signal adsr_02_triggerBuf;
    signal adsr_02_triggerValueBuf;
    bool adsr_02_setupDone;
    UInt noise_tilde_03_nz_state[4] = { };
    bool noise_tilde_03_setupDone;
    number floatnum_08_stored;
    number floatnum_09_stored;
    Int numberobj_08_currentFormat;
    number numberobj_08_lastValue;
    UInt random_03_nz_state[4] = { };
    UInt random_04_nz_state[4] = { };
    number floatnum_10_stored;
    number floatnum_11_stored;
    number param_05_lastValue;
    Int numberobj_09_currentFormat;
    number numberobj_09_lastValue;
    signal globaltransport_tempo;
    signal globaltransport_state;
    number stackprotect_count;
    DataRef pink_tilde_01_rowsobj;
    Index _voiceIndex;
    Int _noteNumber;
    Index isMuted;
    indexlist paramInitIndices;
    indexlist paramInitOrder;
    bool _isInitialized = false;
};

static PatcherInterface* creaternbomatic()
{
    return new rnbomatic<EXTERNALENGINE>();
}

#ifndef RNBO_NO_PATCHERFACTORY
extern "C" PatcherFactoryFunctionPtr GetPatcherFactoryFunction()
#else
extern "C" PatcherFactoryFunctionPtr rnbomaticFactoryFunction()
#endif
{
    return creaternbomatic;
}

#ifndef RNBO_NO_PATCHERFACTORY
extern "C" void SetLogger(Logger* logger)
#else
void rnbomaticSetLogger(Logger* logger)
#endif
{
    console = logger;
}

} // end RNBO namespace

