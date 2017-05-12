describe LLRB::JIT do
  describe '.precompile' do
    it 'compiles class method' do
      klass = Class.new
      def klass.hello
        100
      end

      expect {
        LLRB::JIT.precompile(klass, :hello)
      }.to change {
        klass.method(:hello).hash
      }
      klass.hello
    end

    it 'compiles instance method' do
      klass = Class.new
      klass.class_eval do
        def hello; 100; end
      end
      object = klass.new

      expect {
        LLRB::JIT.precompile(object, :hello)
      }.to change {
        object.method(:hello).hash
      }
      object.hello
    end

    it 'is callable multiple times' do
      klass = Class.new
      def klass.hello
        100
      end
      expect(LLRB::JIT.precompile(klass, :hello)).to eq(true)
      expect(LLRB::JIT.precompile(klass, :hello)).to eq(false)
    end
  end

  describe '.precompile_internal' do
    it 'rejects non-Array argument' do
      object = Object.new
      expect {
        LLRB::JIT.send(:precompile_internal, object, Object, :hash, 0, false)
      }.to raise_error(TypeError)
    end
  end
end