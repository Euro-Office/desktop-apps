---
id: 4-5
title: Use Context for Global State
category: State Management
priority: MEDIUM
description: Use Context API for cross-component shared state
---

## Problem

Passing state through many component layers (prop drilling) creates tight coupling and maintenance burden. Solid's Context API provides a clean way to share state across component trees without explicit prop passing.

## Incorrect

```tsx
// ❌ WRONG: Prop drilling through many layers
function App() {
  const [theme, setTheme] = createSignal("light");
  const [user, setUser] = createSignal(null);

  return (
    <Layout
      theme={theme()}
      setTheme={setTheme}
      user={user()}
      setUser={setUser}
    >
      <MainContent
        theme={theme()}
        user={user()}
        setUser={setUser}
      />
    </Layout>
  );
}

function Layout(props) {
  return (
    <div class={props.theme}>
      <Header
        theme={props.theme}
        setTheme={props.setTheme}
        user={props.user}
      />
      {props.children}
    </div>
  );
}
// ... and so on through every component
```

## Correct

```tsx
import { createContext, useContext, createSignal, ParentComponent } from "solid-js";

// ✅ CORRECT: Create typed context
interface ThemeContextValue {
  theme: () => string;
  setTheme: (theme: string) => void;
  toggleTheme: () => void;
}

const ThemeContext = createContext<ThemeContextValue>();

// ✅ CORRECT: Provider component encapsulates state
const ThemeProvider: ParentComponent = (props) => {
  const [theme, setTheme] = createSignal("light");

  const value: ThemeContextValue = {
    theme,
    setTheme,
    toggleTheme: () => setTheme(t => t === "light" ? "dark" : "light")
  };

  return (
    <ThemeContext.Provider value={value}>
      {props.children}
    </ThemeContext.Provider>
  );
};

// ✅ CORRECT: Custom hook with error handling
function useTheme() {
  const context = useContext(ThemeContext);
  if (!context) {
    throw new Error("useTheme must be used within ThemeProvider");
  }
  return context;
}

// ✅ CORRECT: Clean component hierarchy
function App() {
  return (
    <ThemeProvider>
      <Layout>
        <MainContent />
      </Layout>
    </ThemeProvider>
  );
}

function ThemeToggle() {
  const { theme, toggleTheme } = useTheme();

  return (
    <button onClick={toggleTheme}>
      Current: {theme()}
    </button>
  );
}
```

### With Stores for Complex State

```tsx
import { createContext, useContext, ParentComponent } from "solid-js";
import { createStore } from "solid-js/store";

interface AuthState {
  user: User | null;
  isAuthenticated: boolean;
  permissions: string[];
}

interface AuthContextValue {
  state: AuthState;
  login: (credentials: Credentials) => Promise<void>;
  logout: () => void;
  hasPermission: (permission: string) => boolean;
}

const AuthContext = createContext<AuthContextValue>();

const AuthProvider: ParentComponent = (props) => {
  const [state, setState] = createStore<AuthState>({
    user: null,
    isAuthenticated: false,
    permissions: []
  });

  const login = async (credentials: Credentials) => {
    const response = await api.login(credentials);
    setState({
      user: response.user,
      isAuthenticated: true,
      permissions: response.permissions
    });
  };

  const logout = () => {
    setState({
      user: null,
      isAuthenticated: false,
      permissions: []
    });
  };

  const hasPermission = (permission: string) => {
    return state.permissions.includes(permission);
  };

  return (
    <AuthContext.Provider value={{ state, login, logout, hasPermission }}>
      {props.children}
    </AuthContext.Provider>
  );
};

function useAuth() {
  const context = useContext(AuthContext);
  if (!context) throw new Error("useAuth requires AuthProvider");
  return context;
}
```

### Multiple Contexts

```tsx
// ✅ CORRECT: Compose providers for separation of concerns
function App() {
  return (
    <AuthProvider>
      <ThemeProvider>
        <I18nProvider>
          <NotificationProvider>
            <Router />
          </NotificationProvider>
        </I18nProvider>
      </ThemeProvider>
    </AuthProvider>
  );
}

// Helper component for cleaner nesting
const Providers: ParentComponent = (props) => (
  <AuthProvider>
    <ThemeProvider>
      <I18nProvider>
        {props.children}
      </I18nProvider>
    </ThemeProvider>
  </AuthProvider>
);
```

### Default Context Values

```tsx
// Option 1: Undefined context (require provider)
const StrictContext = createContext<Value>();  // undefined if no provider

// Option 2: Default value (works without provider)
const OptionalContext = createContext<Value>({
  theme: "light",
  setTheme: () => {}  // no-op default
});

// Usage depends on whether provider is optional
function Component() {
  const ctx = useContext(OptionalContext);  // Always defined
  // vs
  const ctx = useContext(StrictContext);    // May be undefined
}
```

## Context Best Practices

1. **Type Safety**: Always define context value types.

2. **Custom Hooks**: Wrap `useContext` in a hook that throws if context is missing.

3. **Stable Values**: Include signals/setters directly, not derived objects.

4. **Split by Concern**: Separate auth, theme, i18n into different contexts.

5. **Colocate Provider**: Place provider at the lowest common ancestor.

## When to Use Context

| Scenario | Use Context? |
| -------- | ------------ |
| Theme, locale, auth state | Yes |
| Form state in multi-step wizard | Yes |
| Global UI state (modals, toasts) | Yes |
| Local component state | No |
| Parent-child (1-2 levels) | Props often simpler |

## Related Rules

- [2-5: Component Composition](2-5-component-composition.md) - Alternative patterns
- [4-1: Signals vs Stores](4-1-signals-vs-stores.md) - State in context
