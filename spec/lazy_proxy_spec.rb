require 'spec_helper'

RSpec.describe LazyProxy do
  subject { LazyProxy }

  it 'exists' do
    expect(LazyProxy).to be_a Class
  end

  describe 'memory management' do
    let(:aux_class) do
      Class.new
    end

    let(:wrapper) { subject.new(aux_class.new) }

    before do
      wrapper = wrapper()
      100.times { wrapper.__setobj__(aux_class.new) }
    end

    it 'gcs old dependencies' do
      GC.start
      expect(ObjectSpace.each_object(aux_class).count).to eq 1
    end
  end

  describe '.new' do
    it 'allows to set a dependency' do
      dep = 'object'
      wrapper = subject.new(dep)
      expect(wrapper.__getobj__).to be dep
    end

    it 'allows to set a dependency as a block' do
      dep = 'object'
      wrapper = subject.new { dep }
      expect(wrapper.__getobj__).to be dep
    end

    it 'arg takes priority' do
      dep = 'object'
      priority_dep = 'chimichanga'
      wrapper = subject.new(priority_dep) { dep }
      expect(wrapper.__getobj__).to be priority_dep
    end

    it 'executes the block once' do
      dep = 'object'
      proc = proc { dep }
      wrapper = subject.new(&proc)
      expect(proc).to receive(:call).once
      wrapper.__getobj__
      wrapper.__getobj__
    end

    it 'allows to leave a dependency unset' do
      wrapper = subject.new
      expect(wrapper.__getobj__).to be_nil
    end
  end

  describe '#__getobj__' do
    let(:dep) { 'object' }
    subject { LazyProxy.new(dep) }

    it 'allows to get a dependency' do
      expect(subject.__getobj__).to be dep
    end

    it 'is always the same dependency' do
      expect(subject.__getobj__).to be subject.__getobj__
    end
  end

  describe '#__setobj__' do
    context 'dependency not previously set' do
      subject { LazyProxy.new }

      it 'allows to set a dependency' do
        dep = 'object'
        subject.__setobj__(dep)
        expect(subject.__getobj__).to be dep
      end

      it 'allows to set a dependency' do
        dep = 'object'
        subject.__setobj__(dep)
        expect(subject.__getobj__).to be dep
      end

      it 'allows to set a dependency as a block' do
        dep = 'object'
        subject.__setobj__ { dep }
        expect(subject.__getobj__).to be dep
      end

      it 'executes the block once' do
        dep = 'object'
        proc = proc { dep }
        subject.__setobj__(&proc)
        expect(proc).to receive(:call).once
        subject.__getobj__
        subject.__getobj__
      end

      it 'arg takes priority' do
        dep = 'object'
        priority_dep = 'chimichanga'
        subject.__setobj__(priority_dep) { dep }
        expect(subject.__getobj__).to be priority_dep
      end

      it 'allows to leave a dependency unset' do
        subject.__setobj__
        expect(subject.__getobj__).to be_nil
      end
    end

    context 'dependency previously set' do
      subject { LazyProxy.new('chimichanga') }

      it 'allows to set a dependency' do
        dep = 'object'
        subject.__setobj__(dep)
        expect(subject.__getobj__).to be dep
      end

      it 'allows to set a dependency as a block' do
        dep = 'object'
        subject.__setobj__ { dep }
        expect(subject.__getobj__).to be dep
      end

      it 'executes the block once' do
        dep = 'object'
        proc = proc { dep }
        subject.__setobj__(&proc)
        expect(proc).to receive(:call).once
        subject.__getobj__
        subject.__getobj__
      end

      it 'arg takes priority' do
        dep = 'object'
        priority_dep = 'chimichanga'
        subject.__setobj__(priority_dep) { dep }
        expect(subject.__getobj__).to be priority_dep
      end

      it 'allows to leave a dependency unset' do
        subject.__setobj__
        expect(subject.__getobj__).to be_nil
      end
    end

    context 'dependency previously set as a block' do
      subject { LazyProxy.new('chimichanga') { 'object' } }

      it 'allows to set a dependency' do
        dep = 'object'
        subject.__setobj__(dep)
        expect(subject.__getobj__).to be dep
      end

      it 'allows to set a dependency as a block' do
        dep = 'object'
        subject.__setobj__ { dep }
        expect(subject.__getobj__).to be dep
      end

      it 'executes the block once' do
        dep = 'object'
        proc = proc { dep }
        subject.__setobj__(&proc)
        expect(proc).to receive(:call).once
        subject.__getobj__
        subject.__getobj__
      end

      it 'arg takes priority' do
        dep = 'object'
        priority_dep = 'chimichanga'
        subject.__setobj__(priority_dep) { dep }
        expect(subject.__getobj__).to be priority_dep
      end

      it 'allows to leave a dependency unset' do
        subject.__setobj__
        expect(subject.__getobj__).to be_nil
      end
    end
  end

  describe '#method_missing' do
    let(:aux_class) do
      Class.new do
        def pub
          :pub
        end

        private

        def priv
          :priv
        end
      end
    end

    it 'works' do
      proxy = LazyProxy.new(aux_class.new)
      expect(proxy.pub).to eq :pub
    end

    it 'works with blocks' do
      proxy = LazyProxy.new { aux_class.new }
      expect(proxy.pub).to eq :pub
    end

    it 'fails for private methods' do
      proxy = LazyProxy.new(aux_class.new)
      expect { proxy.priv }.to raise_error NoMethodError
    end
  end

  describe '#send' do
    let(:aux_class) do
      Class.new do
        def pub(*args)
          args
        end

        private

        def priv(*args)
          args
        end
      end
    end

    it 'works' do
      proxy = LazyProxy.new(aux_class.new)
      expect(proxy.send(:pub, :a)).to eq [:a]
    end

    it 'works with blocks' do
      proxy = LazyProxy.new { aux_class.new }
      expect(proxy.send(:pub, :a)).to eq [:a]
    end

    it 'works for private methods' do
      proxy = LazyProxy.new(aux_class.new)
      expect(proxy.send(:priv, :a)).to eq [:a]
    end
  end

  describe '#public_send' do
    let(:aux_class) do
      Class.new do
        def pub(*args)
          args
        end

        private

        def priv(*args)
          args
        end
      end
    end

    it 'works' do
      proxy = LazyProxy.new(aux_class.new)
      expect(proxy.public_send(:pub, :a)).to eq [:a]
    end

    it 'works with blocks' do
      proxy = LazyProxy.new { aux_class.new }
      expect(proxy.public_send(:pub, :a)).to eq [:a]
    end

    it 'works for private methods' do
      proxy = LazyProxy.new(aux_class.new)
      expect { proxy.public_send(:priv, :a) }.to raise_error NoMethodError
    end
  end

  describe '#inspect' do
    context 'object' do
      let(:object) { :object }

      it 'returns a formatted representation' do
        proxy = LazyProxy.new(object)
        expect(proxy.inspect).to eq "#<LazyProxy: #{object.inspect}>"
      end
    end

    context 'block' do
      let(:block) { proc { :object } }

      it 'returns a formatted representation' do
        prx = LazyProxy.new(&block)
        expect(prx.inspect).to eq "#<LazyProxy: #{block.inspect} (unresolved)>"
      end

      it 'representation containing the resolved value once resolved' do
        proxy = LazyProxy.new(&block)
        proxy.__getobj__
        expect(proxy.inspect).to eq "#<LazyProxy: #{block.call.inspect}>"
      end
    end
  end

  describe '#enum_for' do
    it 'forwards to the wrapped object' do
      prx = LazyProxy.new { 'object' }
      expect(prx.enum_for(:each_byte).to_a).to eq([111, 98, 106, 101, 99, 116])
    end
  end

  describe '#to_enum' do
    it 'forwards to the wrapped object' do
      proxy = LazyProxy.new { 1..3 }
      expect(proxy.to_enum.to_a).to eq([1, 2, 3])
    end
  end

  describe '#to_s' do
    it 'forwards to the wrapped object' do
      proxy = LazyProxy.new { :object }
      expect(proxy.to_s).to eq 'object'
    end
  end

  describe '#!' do
    it 'forwards to the wrapped object' do
      proxy1 = LazyProxy.new { false }
      expect(!proxy1).to be true
      proxy2 = LazyProxy.new { true }
      expect(!proxy2).to be false
    end
  end

  describe '#!=' do
    it 'forwards to the wrapped object' do
      proxy = LazyProxy.new { :object }
      expect(proxy != :object2).to be true
      expect(proxy != :object).to be false
    end
  end

  describe '#==' do
    it 'forwards to the wrapped object' do
      proxy = LazyProxy.new { :object }
      expect(proxy == :object).to be true
      expect(proxy == :object2).to be false
    end
  end

  describe '#===' do
    # rubocop:disable Style/CaseEquality
    it 'forwards to the wrapped object' do
      proxy = LazyProxy.new { 1..3 }
      expect(proxy === 2).to be true
      expect(proxy === 4).to be false
    end
  end

  describe '#=~' do
    it 'forwards to the wrapped object' do
      proxy = LazyProxy.new { 'object' }
      expect(proxy =~ /bje/).to eq 1
      expect(proxy =~ /aje/).to be_nil
    end
  end

  describe '#nil?' do
    it 'forwards to the wrapped object' do
      proxy = LazyProxy.new { nil }
      expect(proxy.nil?).to be true
      proxy.__setobj__(:notnil)
      expect(proxy.nil?).to be false
    end
  end

  describe '#dup' do
    it 'dups and copies over the content' do
      proxy = LazyProxy.new { :object }
      expect(proxy.dup.to_s).to eq 'object'
    end
  end

  describe '#reset' do
    it 'unmemoizes the resolved value from the block' do
      dep = 'object'
      block = proc { dep }
      proxy = LazyProxy.new(&block)
      expect(block).to receive(:call).twice
      proxy.__getobj__
      proxy.__reset__
      proxy.__getobj__
    end

    it 'fails if it is not initialized with a block' do
      dep = 'object'
      proxy = LazyProxy.new(dep)
      expect { proxy.__reset__ }.to raise_error(ArgumentError)
    end
  end

  describe '#respond_to?' do
    let(:aux_class) do
      Class.new do
        def pub
          :pub
        end

        private

        def priv
          :priv
        end
      end
    end

    context 'argument' do
      context 'include private = false' do
        it 'returns true when wrapped obj responds to the public method' do
          expect(LazyProxy.new(aux_class.new).respond_to?(:pub)).to be true
        end

        it 'returns false when wrapped obj responds to the private method' do
          expect(LazyProxy.new(aux_class.new).respond_to?(:priv)).to be false
        end
      end

      context 'include private = true' do
        it 'returns true when wrapped obj responds to the public method' do
          expect(LazyProxy.new(aux_class.new).respond_to?(:pub, true))
            .to be true
        end

        it 'returns true when wrapped obj responds to the private method' do
          expect(LazyProxy.new(aux_class.new).respond_to?(:priv, true))
            .to be true
        end
      end
    end

    context 'block' do
      context 'include private = false' do
        it 'returns true when wrapped obj responds to the public method' do
          expect((LazyProxy.new { aux_class.new }).respond_to?(:pub))
            .to be true
        end

        it 'returns false when wrapped obj responds to the private method' do
          expect((LazyProxy.new { aux_class.new }).respond_to?(:priv))
            .to be false
        end
      end

      context 'include private = true' do
        it 'returns true when wrapped obj responds to the public method' do
          expect((LazyProxy.new { aux_class.new }).respond_to?(:pub, true))
            .to be true
        end

        it 'returns true when wrapped obj responds to the private method' do
          expect((LazyProxy.new { aux_class.new }).respond_to?(:priv, true))
            .to be true
        end
      end
    end
  end
end
